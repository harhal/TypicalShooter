#include "Shooting/ReloadGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "Shooting/AmmoAttributeSet.h"
#include "Shooting/AttackAttributeSet.h"
#include "Shooting/ShootingTags.h"
#include "TimerManager.h"

UReloadGameplayAbility::UReloadGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

bool UReloadGameplayAbility::TryReload(UAbilitySystemComponent& Asc, const EAttackSlot Slot)
{
	FGameplayAbilitySpecHandle Handle;
	return FindForSlot(Asc, Slot, &Handle) && Asc.TryActivateAbility(Handle);
}

const UReloadGameplayAbility* UReloadGameplayAbility::FindForSlot(const UAbilitySystemComponent& Asc, const EAttackSlot Slot,
	FGameplayAbilitySpecHandle* OutHandle)
{
	for (const FGameplayAbilitySpec& Spec : Asc.GetActivatableAbilities())
	{
		const UReloadGameplayAbility* Reload = Cast<UReloadGameplayAbility>(Spec.GetPrimaryInstance());
		Reload = Reload ? Reload : Cast<UReloadGameplayAbility>(Spec.Ability);
		if (Reload && Reload->Slot == Slot)
		{
			if (OutHandle)
			{
				*OutHandle = Spec.Handle;
			}
			return Reload;
		}
	}
	return nullptr;
}

void UReloadGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	UAbilitySystemComponent* Asc = GetAbilitySystemComponentFromActorInfo();
	if (!HasAuthority(&ActivationInfo) || !Asc)
	{
		Finish();
		return;
	}

	const FAttackStats Stats = UAttackAttributeSet::ReadStats(*Asc, Slot);
	if (Stats.Ammo >= Stats.ClipSize || GetReserve(*Asc) <= 0)
	{
		Finish();
		return;
	}

	Asc->AddLooseGameplayTag(ShootingTags::GetReloadingTag(static_cast<uint8>(Slot)), 1, EGameplayTagReplicationState::TagOnly);
	bReloadingTagAdded = true;
	if (Stats.ReloadTime <= 0.f)
	{
		CompleteReload();
		return;
	}
	GetWorld()->GetTimerManager().SetTimer(Timer, FTimerDelegate::CreateUObject(this, &UReloadGameplayAbility::CompleteReload),
		Stats.ReloadTime, false);
}

void UReloadGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const bool bReplicateEndAbility, const bool bWasCancelled)
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(Timer);
	}
	UAbilitySystemComponent* Asc = GetAbilitySystemComponentFromActorInfo();
	if (bReloadingTagAdded && Asc)
	{
		Asc->RemoveLooseGameplayTag(ShootingTags::GetReloadingTag(static_cast<uint8>(Slot)), 1, EGameplayTagReplicationState::TagOnly);
	}
	bReloadingTagAdded = false;
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UReloadGameplayAbility::CompleteReload()
{
	if (UAbilitySystemComponent* Asc = GetAbilitySystemComponentFromActorInfo())
	{
		const FAttackStats Stats = UAttackAttributeSet::ReadStats(*Asc, Slot);
		const int32 Loaded = FMath::Min(Stats.ClipSize - Stats.Ammo, GetReserve(*Asc));
		if (Loaded > 0)
		{
			Asc->SetNumericAttributeBase(UAttackAttributeSet::GetAttribute(Slot, EAttackAttribute::Ammo), Stats.Ammo + Loaded);
			const FGameplayAttribute Reserve = GetReserveAttribute(*Asc);
			if (Reserve.IsValid())
			{
				Asc->SetNumericAttributeBase(Reserve, Asc->GetNumericAttribute(Reserve) - Loaded);
			}
		}
	}
	Finish();
}

FGameplayAttribute UReloadGameplayAbility::GetReserveAttribute(const UAbilitySystemComponent& Asc) const
{
	const FGameplayAttribute Attribute = UAmmoAttributeSet::GetReserveAttribute(AmmoType);
	return Attribute.IsValid() && Asc.HasAttributeSetForAttribute(Attribute) ? Attribute : FGameplayAttribute();
}

int32 UReloadGameplayAbility::GetReserve(const UAbilitySystemComponent& Asc) const
{
	const FGameplayAttribute Attribute = GetReserveAttribute(Asc);
	return Attribute.IsValid() ? FMath::FloorToInt32(Asc.GetNumericAttribute(Attribute)) : MAX_int32;
}

void UReloadGameplayAbility::Finish()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}
