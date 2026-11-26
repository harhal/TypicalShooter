#include "Loot/LootPowerParams.h"

FAttackStatBonus FLootAttackStatRange::Roll(FRandomStream& Stream, const float LevelMultiplier) const
{
	FAttackStatBonus Bonus;
	Bonus.Attribute = Attribute;
	Bonus.Value = Value.Roll(Stream, LevelMultiplier);
	return Bonus;
}

FPowerSourceParams FLootSourceParams::Roll(FRandomStream& Stream, const float LevelMultiplier) const
{
	FPowerSourceParams Params;
	Params.Power = Power.Roll(Stream, LevelMultiplier);
	Params.AmmoType = AmmoType;
	Params.Capacity = Capacity.Roll(Stream, LevelMultiplier);
	Params.CostPerUse = CostPerUse.Roll(Stream, LevelMultiplier);
	Params.RechargeDelay = RechargeDelay.Roll(Stream, LevelMultiplier);
	return Params;
}

FPowerReceiverParams FLootReceiverParams::Roll(FRandomStream& Stream, const float LevelMultiplier) const
{
	FPowerReceiverParams Params;
	Params.Ability = Ability;
	Params.RequiredPower = RequiredPower.Roll(Stream, LevelMultiplier);
	for (const FLootAttackStatRange& Stat : BaseStats)
	{
		Params.BaseStats.Add(Stat.Roll(Stream, LevelMultiplier));
	}
	return Params;
}
