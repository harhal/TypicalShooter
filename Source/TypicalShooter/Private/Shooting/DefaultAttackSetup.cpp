#include "Shooting/DefaultAttackSetup.h"

#include "Shooting/AmmoAttributeSet.h"
#include "Shooting/ShootingTags.h"

UPrimaryAttackAbility::UPrimaryAttackAbility()
{
	Slot = EAttackSlot::Primary;
}

UAlternateAttackAbility::UAlternateAttackAbility()
{
	Slot = EAttackSlot::Alternate;
	Pattern.Offsets = {
		FVector2D(0.f, 0.f),
		FVector2D(3.f, 0.f), FVector2D(1.5f, 2.6f), FVector2D(-1.5f, 2.6f),
		FVector2D(-3.f, 0.f), FVector2D(-1.5f, -2.6f), FVector2D(1.5f, -2.6f) };
}

UDoubleJumpAbility::UDoubleJumpAbility()
{
	Slot = EAttackSlot::Air;
	bSpawnsProjectiles = false;
}

UDefaultAttackStatsEffect::UDefaultAttackStatsEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	// Reserve ammo (R15).
	AddReserve(ShootingTags::Ammo_Bullet, 120.f);
	AddReserve(ShootingTags::Ammo_Particle, 24.f);
}

void UDefaultAttackStatsEffect::AddReserve(const FGameplayTag& AmmoType, const float Value)
{
	FGameplayModifierInfo Modifier;
	Modifier.Attribute = UAmmoAttributeSet::GetReserveAttribute(AmmoType);
	Modifier.ModifierOp = EGameplayModOp::Override;
	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(Value));
	Modifiers.Add(Modifier);
}
