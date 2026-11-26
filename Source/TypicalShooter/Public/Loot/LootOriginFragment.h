// Item fragment: how a generated item was made (LootGeneration R8). Hand-made items don't have it.

#pragma once

#include "CoreMinimal.h"
#include "ItemFragment.h"
#include "LootOriginFragment.generated.h"

class ULootItemTemplate;
class ULootModifier;
class ULootRarity;

USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FLootOrigin_ItemFragment : public FItemFragment
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Loot")
	TObjectPtr<const ULootItemTemplate> Template = nullptr;

	/** Clamped loot level the item was rolled for. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Loot")
	int32 Level = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Loot")
	int32 Seed = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Loot")
	TObjectPtr<const ULootRarity> Rarity = nullptr;

	/** Picked modifiers, in slot order. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Loot")
	TArray<TObjectPtr<const ULootModifier>> Modifiers;
};
