// Loot modifier: one random power threshold step an item can get (LootGeneration R3).

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Loot/LootPowerParams.h"
#include "Loot/LootRange.h"
#include "PowerGrid/PowerEffectsFragment.h"
#include "LootModifier.generated.h"

class UGameplayEffect;

UCLASS(BlueprintType)
class TYPICALSHOOTER_API ULootModifier : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	FPowerThresholdEffect Roll(FRandomStream& Stream, float LevelMultiplier) const;

	/** Rarity slots take only modifiers of exactly their tier. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot", meta = (ClampMin = 1))
	int32 Tier = 1;

	/** Category; a slot takes modifiers whose tag matches its own (Loot.Modifier.Attack.Fire fits Loot.Modifier.Attack). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot", meta = (Categories = "Loot.Modifier"))
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	FLootIntRange Threshold = FLootIntRange(1, 1);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	TArray<FLootAttackStatRange> AttackBonuses;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	TSubclassOf<UGameplayEffect> CharacterEffect;

	/** Line added to the item description; placeholders {Threshold}, {Bonus0}, {Bonus1}, ... */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Text", meta = (MultiLine = true))
	FText DescriptionLine;
};
