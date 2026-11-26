// Name, description and icon of a generated item (LootGeneration R7).

#pragma once

#include "CoreMinimal.h"
#include "ItemFragment.h"
#include "PowerGrid/PowerEffectsFragment.h"

struct FPowerGrid_ItemFragment;
class ULootItemTemplate;
class ULootModifier;
class ULootRarity;

/** A picked modifier with the step it rolled. */
struct FLootRolledModifier
{
	const ULootModifier* Modifier = nullptr;
	FPowerThresholdEffect Step;
};

namespace LootItemText
{
	/** Steps are expected in their final (threshold) order; their modifiers' lines follow the description in that order. */
	TYPICALSHOOTER_API FInfo_ItemFragment MakeInfo(const ULootItemTemplate& Template, const FPowerGrid_ItemFragment& PowerGrid,
		TConstArrayView<FLootRolledModifier> Rolled, int32 Level, const ULootRarity* Rarity);
}
