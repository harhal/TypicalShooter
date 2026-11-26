#include "Loot/LootRandom.h"

int32 LootRandom::PickWeightedIndex(FRandomStream& Stream, TConstArrayView<float> Weights)
{
	float Total = 0.f;
	for (const float Weight : Weights)
	{
		Total += FMath::Max(0.f, Weight);
	}
	if (Total <= 0.f)
	{
		return INDEX_NONE;
	}

	float Roll = Stream.FRandRange(0.f, Total);
	int32 LastPositive = INDEX_NONE;
	for (int32 Index = 0; Index < Weights.Num(); ++Index)
	{
		if (Weights[Index] <= 0.f)
		{
			continue;
		}
		LastPositive = Index;
		if (Roll < Weights[Index])
		{
			return Index;
		}
		Roll -= Weights[Index];
	}
	return LastPositive; // Roll == Total (float edge)
}
