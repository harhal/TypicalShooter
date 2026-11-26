// Item effects switched on by the power fed to the item (power grid R13-R15).

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "ItemFragment.h"
#include "Shooting/ShotModifier.h"
#include "PowerEffectsFragment.generated.h"

/** One step: active while the item gets at least Threshold power (0 = always, even unconnected). */
USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FPowerThresholdEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Grid", meta = (ClampMin = 0))
	int32 Threshold = 0;

	/** Applied to the whole character (health, movement, ...) (R14). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Grid")
	TSubclassOf<UGameplayEffect> CharacterEffect;

	/** Level the character effect is applied with (generated loot: the item level). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Grid", meta = (ClampMin = 1))
	float CharacterEffectLevel = 1.f;

	/** Added to the slot attributes of every receiver the item is connected to (R14, R14a). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Grid")
	TArray<FAttackStatBonus> AttackBonuses;
};

/** Threshold effects of an item; all reached steps are active at once (R13). */
USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FPowerEffects_ItemFragment : public FItemFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Grid")
	TArray<FPowerThresholdEffect> Effects;
};
