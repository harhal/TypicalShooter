// Loot rarity: name, color and the modifier slots an item of this rarity gets (LootGeneration R4).

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "LootRarity.generated.h"

/** One modifier an item gets: a modifier of exactly this tier whose tag matches Tag (empty = any tag). */
USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FLootModifierSlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (ClampMin = 1))
	int32 Tier = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (Categories = "Loot.Modifier"))
	FGameplayTag Tag;
};

UCLASS(BlueprintType)
class TYPICALSHOOTER_API ULootRarity : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rarity")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rarity")
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rarity")
	TArray<FLootModifierSlot> Slots;
};
