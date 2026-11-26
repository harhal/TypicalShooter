// Rarity of an item as the UI shows it (LootGeneration R10).

#pragma once

#include "CoreMinimal.h"

struct FItemRarityDisplay
{
	FText Name;
	FLinearColor Color = FLinearColor::White;
};
