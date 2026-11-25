#include "Tests/ShootingTestEffects.h"

#include "Shooting/AttackAttributeSet.h"

UTestPrimaryDamageBonusEffect::UTestPrimaryDamageBonusEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;
	FGameplayModifierInfo Modifier;
	Modifier.Attribute = UAttackAttributeSet::GetAttribute(EAttackSlot::Primary, EAttackAttribute::DamagePhysical);
	Modifier.ModifierOp = EGameplayModOp::Additive;
	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(5.f));
	Modifiers.Add(Modifier);
}
