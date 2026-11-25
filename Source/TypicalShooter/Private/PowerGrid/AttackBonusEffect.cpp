#include "PowerGrid/AttackBonusEffect.h"

#include "Shooting/AttackAttributeSet.h"

UAttackBonusEffect::UAttackBonusEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;
	for (int32 SlotIndex = 0; SlotIndex <= static_cast<int32>(EAttackSlot::Air); ++SlotIndex)
	{
		for (int32 Index = 0; Index < static_cast<int32>(EAttackAttribute::Count); ++Index)
		{
			const EAttackSlot Slot = static_cast<EAttackSlot>(SlotIndex);
			const EAttackAttribute Attribute = static_cast<EAttackAttribute>(Index);
			FSetByCallerFloat SetByCaller;
			SetByCaller.DataName = GetMagnitudeName(Slot, Attribute);

			FGameplayModifierInfo Modifier;
			Modifier.Attribute = UAttackAttributeSet::GetAttribute(Slot, Attribute);
			Modifier.ModifierOp = EGameplayModOp::Additive;
			Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
			Modifiers.Add(Modifier);
		}
	}
}

FName UAttackBonusEffect::GetMagnitudeName(const EAttackSlot Slot, const EAttackAttribute Attribute)
{
	return FName(*FString::Printf(TEXT("AttackBonus.%d.%d"), static_cast<int32>(Slot), static_cast<int32>(Attribute)));
}
