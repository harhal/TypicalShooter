// Random helpers shared by loot generation.

#pragma once

#include "CoreMinimal.h"

namespace LootRandom
{
	/** Index picked proportionally to Weights (non-positive weights never win); INDEX_NONE if nothing can win. */
	TYPICALSHOOTER_API int32 PickWeightedIndex(FRandomStream& Stream, TConstArrayView<float> Weights);
}
