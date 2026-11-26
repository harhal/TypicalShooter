// Picks modifiers from a template pool for the slots of a rarity (LootGeneration R4).

#pragma once

#include "CoreMinimal.h"

struct FLootModifierEntry;
class ULootModifier;
class ULootRarity;

namespace LootModifierPicker
{
	/** One modifier per slot (tier equal, tag matching, by weight, no repeats); slots without a candidate are skipped. */
	TYPICALSHOOTER_API TArray<const ULootModifier*> Pick(TConstArrayView<FLootModifierEntry> Pool, const ULootRarity* Rarity, FRandomStream& Stream);
}
