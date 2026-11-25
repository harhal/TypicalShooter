#include "Shooting/ShotDamageEffect.h"

#include "Shooting/HealthAttributeSet.h"
#include "Shooting/ShootingTags.h"

UShotDamageEffect::UShotDamageEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	for (const FGameplayTag& Type : ShootingTags::GetDamageTypes())
	{
		FSetByCallerFloat SetByCaller;
		SetByCaller.DataTag = Type;

		FGameplayModifierInfo Modifier;
		Modifier.Attribute = UHealthAttributeSet::GetIncomingDamageAttribute();
		Modifier.ModifierOp = EGameplayModOp::Additive;
		Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
		Modifiers.Add(Modifier);
	}
}
