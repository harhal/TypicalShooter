#include "Shooting/AmmoAttributeSet.h"

#include "Net/UnrealNetwork.h"
#include "Shooting/ShootingTags.h"

#define TS_AMMO_ATTRIBUTE(Name) \
	FGameplayAttribute(FindFieldChecked<FProperty>(UAmmoAttributeSet::StaticClass(), GET_MEMBER_NAME_CHECKED(UAmmoAttributeSet, Name)))

FGameplayAttribute UAmmoAttributeSet::GetReserveAttribute(const FGameplayTag& AmmoType)
{
	static const TMap<FGameplayTag, FGameplayAttribute> Attributes = {
		{ ShootingTags::Ammo_Bullet, TS_AMMO_ATTRIBUTE(Reserve_Bullet) },
		{ ShootingTags::Ammo_Battery, TS_AMMO_ATTRIBUTE(Reserve_Battery) },
		{ ShootingTags::Ammo_Grenade, TS_AMMO_ATTRIBUTE(Reserve_Grenade) },
		{ ShootingTags::Ammo_Radiator, TS_AMMO_ATTRIBUTE(Reserve_Radiator) },
		{ ShootingTags::Ammo_Particle, TS_AMMO_ATTRIBUTE(Reserve_Particle) } };
	const FGameplayAttribute* Attribute = Attributes.Find(AmmoType);
	return Attribute ? *Attribute : FGameplayAttribute();
}

#undef TS_AMMO_ATTRIBUTE

void UAmmoAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	NewValue = FMath::Max(FMath::FloorToFloat(NewValue), 0.f); // whole rounds, never negative
}

void UAmmoAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	NewValue = FMath::Max(FMath::FloorToFloat(NewValue), 0.f);
}

void UAmmoAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(UAmmoAttributeSet, Reserve_Bullet, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAmmoAttributeSet, Reserve_Battery, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAmmoAttributeSet, Reserve_Grenade, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAmmoAttributeSet, Reserve_Radiator, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAmmoAttributeSet, Reserve_Particle, COND_None, REPNOTIFY_Always);
}

void UAmmoAttributeSet::OnRep_Reserve_Bullet(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAmmoAttributeSet, Reserve_Bullet, OldValue);
}

void UAmmoAttributeSet::OnRep_Reserve_Battery(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAmmoAttributeSet, Reserve_Battery, OldValue);
}

void UAmmoAttributeSet::OnRep_Reserve_Grenade(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAmmoAttributeSet, Reserve_Grenade, OldValue);
}

void UAmmoAttributeSet::OnRep_Reserve_Radiator(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAmmoAttributeSet, Reserve_Radiator, OldValue);
}

void UAmmoAttributeSet::OnRep_Reserve_Particle(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAmmoAttributeSet, Reserve_Particle, OldValue);
}
