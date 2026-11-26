// Rolls a concrete item manifest from a loot template (LootGeneration R1, R6, R8).

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemManifest.h"

class ULootItemTemplate;

struct FLootGenerationParams
{
	int32 Level = 1;
	/** Everything random about the item comes from a stream with this seed. */
	int32 Seed = 0;
};

namespace LootGenerator
{
	TYPICALSHOOTER_API FInventoryItemManifest Generate(const ULootItemTemplate& Template, const FLootGenerationParams& Params);
}
