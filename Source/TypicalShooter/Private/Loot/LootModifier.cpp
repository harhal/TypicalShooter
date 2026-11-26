#include "Loot/LootModifier.h"

FPowerThresholdEffect ULootModifier::Roll(FRandomStream& Stream, const float LevelMultiplier) const
{
	FPowerThresholdEffect Step;
	Step.Threshold = Threshold.Roll(Stream, LevelMultiplier);
	Step.CharacterEffect = CharacterEffect;
	for (const FLootAttackStatRange& Bonus : AttackBonuses)
	{
		Step.AttackBonuses.Add(Bonus.Roll(Stream, LevelMultiplier));
	}
	return Step;
}
