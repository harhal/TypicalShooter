#include "Shooting/AttackAttributeSet.h"

#include "Net/UnrealNetwork.h"
#include "Shooting/ShootingTags.h"

namespace AttackAttributeSetPrivate
{
	constexpr int32 NumSlots = 2;
	constexpr int32 NumAttributes = static_cast<int32>(EAttackAttribute::Count);

	using FTable = TStaticArray<TStaticArray<FGameplayAttribute, NumAttributes>, NumSlots>;

	/** Attributes by [slot][parameter], found by property name "<Slot>_<Parameter>". */
	const FTable& GetTable()
	{
		static const FTable Table = []
		{
			FTable Result;
			const UEnum* SlotEnum = StaticEnum<EAttackSlot>();
			const UEnum* AttributeEnum = StaticEnum<EAttackAttribute>();
			for (int32 Slot = 0; Slot < NumSlots; ++Slot)
			{
				for (int32 Index = 0; Index < NumAttributes; ++Index)
				{
					const FString Name = SlotEnum->GetNameStringByValue(Slot) + TEXT("_") + AttributeEnum->GetNameStringByValue(Index);
					FProperty* Property = FindFProperty<FProperty>(UAttackAttributeSet::StaticClass(), *Name);
					checkf(Property, TEXT("UAttackAttributeSet is missing attribute %s"), *Name);
					Result[Slot][Index] = FGameplayAttribute(Property);
				}
			}
			return Result;
		}();
		return Table;
	}

	/** Slot and parameter of an attack attribute; false if Attribute is not one. */
	bool Find(const FGameplayAttribute& Attribute, EAttackSlot& OutSlot, EAttackAttribute& OutAttribute)
	{
		const FTable& Table = GetTable();
		for (int32 Slot = 0; Slot < NumSlots; ++Slot)
		{
			for (int32 Index = 0; Index < NumAttributes; ++Index)
			{
				if (Table[Slot][Index] == Attribute)
				{
					OutSlot = static_cast<EAttackSlot>(Slot);
					OutAttribute = static_cast<EAttackAttribute>(Index);
					return true;
				}
			}
		}
		return false;
	}

	float DefaultValue(const EAttackAttribute Attribute)
	{
		switch (Attribute)
		{
		case EAttackAttribute::FireEndDistance: return 10000.f;
		case EAttackAttribute::ClipSize: return 1.f;
		case EAttackAttribute::PatternScale: return 1.f;
		case EAttackAttribute::ShotsPerBurst: return 1.f;
		case EAttackAttribute::ProjectileSpeed: return 5000.f;
		case EAttackAttribute::DamageLoss: return 1.f;
		default: return 0.f;
		}
	}
}

UAttackAttributeSet::UAttackAttributeSet()
{
	using namespace AttackAttributeSetPrivate;
	for (int32 Slot = 0; Slot < NumSlots; ++Slot)
	{
		for (int32 Index = 0; Index < NumAttributes; ++Index)
		{
			FGameplayAttributeData* Data = GetTable()[Slot][Index].GetGameplayAttributeData(this);
			const float Value = DefaultValue(static_cast<EAttackAttribute>(Index));
			Data->SetBaseValue(Value);
			Data->SetCurrentValue(Value);
		}
	}
}

FGameplayAttribute UAttackAttributeSet::GetAttribute(const EAttackSlot Slot, const EAttackAttribute Attribute)
{
	const int32 Index = static_cast<int32>(Attribute);
	if (Index < 0 || Index >= AttackAttributeSetPrivate::NumAttributes)
	{
		return FGameplayAttribute();
	}
	return AttackAttributeSetPrivate::GetTable()[static_cast<int32>(Slot)][Index];
}

FAttackStats UAttackAttributeSet::ReadStats(const UAbilitySystemComponent& Asc, const EAttackSlot Slot)
{
	const float ClipSize = ClampValue(EAttackAttribute::ClipSize, Asc.GetNumericAttribute(GetAttribute(Slot, EAttackAttribute::ClipSize)), 0.f);
	auto Read = [&Asc, Slot, ClipSize](const EAttackAttribute Attribute)
	{
		return ClampValue(Attribute, Asc.GetNumericAttribute(GetAttribute(Slot, Attribute)), ClipSize);
	};

	FAttackStats Stats;
	Stats.FireStartDistance = Read(EAttackAttribute::FireStartDistance);
	Stats.FireEndDistance = FMath::Max(Read(EAttackAttribute::FireEndDistance), Stats.FireStartDistance);
	Stats.Ammo = FMath::FloorToInt32(Read(EAttackAttribute::Ammo));
	Stats.ClipSize = FMath::RoundToInt32(ClipSize);
	Stats.PatternScale = Read(EAttackAttribute::PatternScale);
	Stats.ShotsPerBurst = FMath::RoundToInt32(Read(EAttackAttribute::ShotsPerBurst));
	Stats.ShotInterval = Read(EAttackAttribute::ShotInterval);
	Stats.BurstInterval = Read(EAttackAttribute::BurstInterval);
	Stats.ReloadTime = Read(EAttackAttribute::ReloadTime);
	Stats.ProjectileSpeed = Read(EAttackAttribute::ProjectileSpeed);
	Stats.GravityScale = Read(EAttackAttribute::GravityScale);
	Stats.DamageLoss = Read(EAttackAttribute::DamageLoss);

	// Damage attributes follow the order of ShootingTags::GetDamageTypes().
	const TConstArrayView<FGameplayTag> DamageTypes = ShootingTags::GetDamageTypes();
	for (int32 TypeIndex = 0; TypeIndex < DamageTypes.Num(); ++TypeIndex)
	{
		const float Amount = Read(static_cast<EAttackAttribute>(static_cast<int32>(EAttackAttribute::DamagePhysical) + TypeIndex));
		if (Amount > 0.f)
		{
			Stats.Damage.Amounts.Add(DamageTypes[TypeIndex], Amount);
		}
	}
	return Stats;
}

void UAttackAttributeSet::AddToStats(FAttackStats& Stats, const EAttackAttribute Attribute, const float Value)
{
	auto Add = [Attribute, Value, &Stats](float& Field) { Field = ClampValue(Attribute, Field + Value, Stats.ClipSize); };
	switch (Attribute)
	{
	case EAttackAttribute::FireStartDistance: Add(Stats.FireStartDistance); break;
	case EAttackAttribute::FireEndDistance: Add(Stats.FireEndDistance); break;
	case EAttackAttribute::PatternScale: Add(Stats.PatternScale); break;
	case EAttackAttribute::ShotInterval: Add(Stats.ShotInterval); break;
	case EAttackAttribute::BurstInterval: Add(Stats.BurstInterval); break;
	case EAttackAttribute::ReloadTime: Add(Stats.ReloadTime); break;
	case EAttackAttribute::ProjectileSpeed: Add(Stats.ProjectileSpeed); break;
	case EAttackAttribute::GravityScale: Add(Stats.GravityScale); break;
	case EAttackAttribute::DamageLoss: Add(Stats.DamageLoss); break;
	case EAttackAttribute::Ammo:
	case EAttackAttribute::ClipSize:
	case EAttackAttribute::ShotsPerBurst:
	case EAttackAttribute::Count:
		break; // not per-shot parameters
	default:
	{
		const int32 TypeIndex = static_cast<int32>(Attribute) - static_cast<int32>(EAttackAttribute::DamagePhysical);
		const FGameplayTag Type = ShootingTags::GetDamageTypes()[TypeIndex];
		float& Amount = Stats.Damage.Amounts.FindOrAdd(Type);
		Amount = FMath::Max(Amount + Value, 0.f);
		break;
	}
	}
	Stats.FireEndDistance = FMath::Max(Stats.FireEndDistance, Stats.FireStartDistance);
}

float UAttackAttributeSet::ClampValue(const EAttackAttribute Attribute, const float Value, const float ClipSize)
{
	switch (Attribute)
	{
	case EAttackAttribute::Ammo:
		return FMath::Clamp(FMath::FloorToFloat(Value), 0.f, FMath::Max(ClipSize, 1.f));
	case EAttackAttribute::ClipSize:
	case EAttackAttribute::ShotsPerBurst:
		return FMath::Max(FMath::RoundToFloat(Value), 1.f);
	case EAttackAttribute::DamageLoss:
		return FMath::Clamp(Value, 0.f, 1.f);
	case EAttackAttribute::ProjectileSpeed:
		return FMath::Max(Value, 1.f);
	case EAttackAttribute::GravityScale:
		return Value;
	default:
		return FMath::Max(Value, 0.f);
	}
}

void UAttackAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	ClampAttribute(Attribute, NewValue);
}

void UAttackAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	ClampAttribute(Attribute, NewValue);
}

void UAttackAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, const float OldValue, const float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	EAttackSlot Slot;
	EAttackAttribute Parameter;
	if (!AttackAttributeSetPrivate::Find(Attribute, Slot, Parameter) || Parameter != EAttackAttribute::ClipSize)
	{
		return;
	}
	// A smaller clip drops the ammo that no longer fits.
	const FGameplayAttribute Ammo = GetAttribute(Slot, EAttackAttribute::Ammo);
	UAbilitySystemComponent* Asc = GetOwningAbilitySystemComponent();
	if (Asc && Ammo.GetNumericValue(this) > NewValue)
	{
		Asc->SetNumericAttributeBase(Ammo, NewValue);
	}
}

void UAttackAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	EAttackSlot Slot;
	EAttackAttribute Parameter;
	if (AttackAttributeSetPrivate::Find(Attribute, Slot, Parameter))
	{
		const float ClipSize = GetAttribute(Slot, EAttackAttribute::ClipSize).GetNumericValue(this);
		NewValue = ClampValue(Parameter, NewValue, ClipSize);
	}
}

void UAttackAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

#define TS_REP_ATTACK_ATTRIBUTE(Name) \
	DOREPLIFETIME_CONDITION_NOTIFY(UAttackAttributeSet, Primary_##Name, COND_None, REPNOTIFY_Always); \
	DOREPLIFETIME_CONDITION_NOTIFY(UAttackAttributeSet, Alternate_##Name, COND_None, REPNOTIFY_Always);

	TS_REP_ATTACK_ATTRIBUTE(FireStartDistance)
	TS_REP_ATTACK_ATTRIBUTE(FireEndDistance)
	TS_REP_ATTACK_ATTRIBUTE(Ammo)
	TS_REP_ATTACK_ATTRIBUTE(ClipSize)
	TS_REP_ATTACK_ATTRIBUTE(PatternScale)
	TS_REP_ATTACK_ATTRIBUTE(ShotsPerBurst)
	TS_REP_ATTACK_ATTRIBUTE(ShotInterval)
	TS_REP_ATTACK_ATTRIBUTE(BurstInterval)
	TS_REP_ATTACK_ATTRIBUTE(ReloadTime)
	TS_REP_ATTACK_ATTRIBUTE(ProjectileSpeed)
	TS_REP_ATTACK_ATTRIBUTE(GravityScale)
	TS_REP_ATTACK_ATTRIBUTE(DamageLoss)
	TS_REP_ATTACK_ATTRIBUTE(DamagePhysical)
	TS_REP_ATTACK_ATTRIBUTE(DamageAcid)
	TS_REP_ATTACK_ATTRIBUTE(DamageElectric)
	TS_REP_ATTACK_ATTRIBUTE(DamagePlasma)
	TS_REP_ATTACK_ATTRIBUTE(DamageIce)
	TS_REP_ATTACK_ATTRIBUTE(DamageRadiation)

#undef TS_REP_ATTACK_ATTRIBUTE
}

void UAttackAttributeSet::OnRep_Primary_Ammo(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAttackAttributeSet, Primary_Ammo, OldValue);
}

void UAttackAttributeSet::OnRep_Primary_ClipSize(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAttackAttributeSet, Primary_ClipSize, OldValue);
}

void UAttackAttributeSet::OnRep_Alternate_Ammo(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAttackAttributeSet, Alternate_Ammo, OldValue);
}

void UAttackAttributeSet::OnRep_Alternate_ClipSize(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAttackAttributeSet, Alternate_ClipSize, OldValue);
}
