// Random value ranges used by loot templates and modifiers.

#pragma once

#include "CoreMinimal.h"
#include "LootRange.generated.h"

/** Integer range [Min, Max]; optionally multiplied by the loot level multiplier (see ULootSettings). */
USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FLootIntRange
{
	GENERATED_BODY()

	FLootIntRange() = default;
	FLootIntRange(const int32 InMin, const int32 InMax, const bool bInScalesWithLevel = false)
		: Min(InMin), Max(InMax), bScalesWithLevel(bInScalesWithLevel) {}

	/** Uniform value in [Min, Max] (order-agnostic), times LevelMultiplier if it scales; rounded, never below 0. */
	int32 Roll(FRandomStream& Stream, float LevelMultiplier) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	int32 Min = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	int32 Max = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	bool bScalesWithLevel = false;
};

/** Float range [Min, Max]; optionally multiplied by the loot level multiplier (see ULootSettings). */
USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FLootFloatRange
{
	GENERATED_BODY()

	FLootFloatRange() = default;
	FLootFloatRange(const float InMin, const float InMax, const bool bInScalesWithLevel = false)
		: Min(InMin), Max(InMax), bScalesWithLevel(bInScalesWithLevel) {}

	/** Uniform value in [Min, Max] (order-agnostic), times LevelMultiplier if it scales; never below 0. */
	float Roll(FRandomStream& Stream, float LevelMultiplier) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	float Min = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	float Max = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	bool bScalesWithLevel = false;
};
