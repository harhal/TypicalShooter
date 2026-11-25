#include "Shooting/DefaultAttackSetup.h"

#include "Shooting/AmmoAttributeSet.h"
#include "Shooting/AttackAttributeSet.h"
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

UDefaultAttackStatsEffect::UDefaultAttackStatsEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	using A = EAttackAttribute;

	// Primary: automatic rifle (held button repeats single-shot bursts).
	constexpr EAttackSlot P = EAttackSlot::Primary;
	AddOverride(P, A::FireStartDistance, 0.f);
	AddOverride(P, A::FireEndDistance, 8000.f);
	AddOverride(P, A::ShotsPerBurst, 1.f);
	AddOverride(P, A::ShotInterval, 0.f);
	AddOverride(P, A::BurstInterval, 0.12f);
	AddOverride(P, A::ProjectileSpeed, 8000.f);
	AddOverride(P, A::GravityScale, 0.f);
	AddOverride(P, A::DamageLoss, 1.f);
	AddOverride(P, A::PatternScale, 1.f);
	AddOverride(P, A::DamagePhysical, 10.f);

	// Alternate: shotgun (seven pellets, slower, shorter range, slight drop).
	constexpr EAttackSlot S = EAttackSlot::Alternate;
	AddOverride(S, A::FireStartDistance, 0.f);
	AddOverride(S, A::FireEndDistance, 3000.f);
	AddOverride(S, A::ShotsPerBurst, 1.f);
	AddOverride(S, A::ShotInterval, 0.f);
	AddOverride(S, A::BurstInterval, 0.8f);
	AddOverride(S, A::ProjectileSpeed, 6000.f);
	AddOverride(S, A::GravityScale, 0.2f);
	AddOverride(S, A::DamageLoss, 1.f);
	AddOverride(S, A::PatternScale, 1.f);
	AddOverride(S, A::DamagePhysical, 6.f);

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

void UDefaultAttackStatsEffect::AddOverride(const EAttackSlot Slot, const EAttackAttribute Attribute, const float Value)
{
	FGameplayModifierInfo Modifier;
	Modifier.Attribute = UAttackAttributeSet::GetAttribute(Slot, Attribute);
	Modifier.ModifierOp = EGameplayModOp::Override;
	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(Value));
	Modifiers.Add(Modifier);
}
