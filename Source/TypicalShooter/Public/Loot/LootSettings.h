// Project-wide loot generation settings (Project Settings -> Game -> Loot).

#pragma once

#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"
#include "Engine/DeveloperSettings.h"
#include "LootSettings.generated.h"

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Loot"))
class TYPICALSHOOTER_API ULootSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	static constexpr int32 MinLevel = 1;
	static constexpr int32 MaxLevel = 100;

	ULootSettings();

	virtual FName GetCategoryName() const override { return TEXT("Game"); }

	static int32 ClampLevel(int32 Level);

	/** Multiplier for ranges that scale with level; the level is clamped to [MinLevel, MaxLevel]. */
	float GetLevelMultiplier(int32 Level) const;

	/** X = loot level, Y = multiplier. Empty curve = 1. */
	UPROPERTY(Config, EditAnywhere, Category = "Level")
	FRuntimeFloatCurve LevelMultiplier;
};
