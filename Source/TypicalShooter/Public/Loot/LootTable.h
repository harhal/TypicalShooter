// Loot table: how many items, of which templates and rarities, a source of loot gives (LootGeneration R5).

#pragma once

#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"
#include "Engine/DataAsset.h"
#include "InventoryItemManifest.h"
#include "Loot/LootRange.h"
#include "LootTable.generated.h"

class ULootItemTemplate;
class ULootRarity;

USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FLootTemplateEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	TObjectPtr<const ULootItemTemplate> Template = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (ClampMin = 0))
	float Weight = 1.f;
};

USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FLootRarityEntry
{
	GENERATED_BODY()

	/** Pick weight at a loot level (X = level, Y = weight); an empty curve = weight 1 at every level. */
	float GetWeight(int32 Level) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	TObjectPtr<const ULootRarity> Rarity = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	FRuntimeFloatCurve WeightByLevel;
};

UCLASS(BlueprintType)
class TYPICALSHOOTER_API ULootTable : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Item manifests for a loot level; the same (table, level, seed) always gives the same items. */
	TArray<FInventoryItemManifest> Roll(int32 Level, int32 Seed) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	FLootIntRange ItemCount = FLootIntRange(1, 3);

	/** Picked by weight for every item; repeats allowed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	TArray<FLootTemplateEntry> Templates;

	/** Picked by level-dependent weight for every item; empty = items without rarity. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	TArray<FLootRarityEntry> Rarities;
};
