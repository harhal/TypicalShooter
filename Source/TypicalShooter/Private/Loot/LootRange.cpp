#include "Loot/LootRange.h"

int32 FLootIntRange::Roll(FRandomStream& Stream, const float LevelMultiplier) const
{
	const int32 Value = Stream.RandRange(FMath::Min(Min, Max), FMath::Max(Min, Max));
	const int32 Scaled = bScalesWithLevel ? FMath::RoundToInt32(Value * LevelMultiplier) : Value;
	return FMath::Max(0, Scaled);
}

float FLootFloatRange::Roll(FRandomStream& Stream, const float LevelMultiplier) const
{
	const float Value = Stream.FRandRange(FMath::Min(Min, Max), FMath::Max(Min, Max));
	const float Scaled = bScalesWithLevel ? Value * LevelMultiplier : Value;
	return FMath::Max(0.f, Scaled);
}
