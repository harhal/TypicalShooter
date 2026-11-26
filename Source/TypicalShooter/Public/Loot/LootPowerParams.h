// Random power grid parameters of a loot template (LootGeneration R1, R2).

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Loot/LootRange.h"
#include "PowerGrid/PowerGridFragment.h"
#include "Shooting/AttackTypes.h"
#include "LootPowerParams.generated.h"

/** Attack attribute with a random value; rolls into FAttackStatBonus. */
USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FLootAttackStatRange
{
	GENERATED_BODY()

	FAttackStatBonus Roll(FRandomStream& Stream, float LevelMultiplier) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	EAttackAttribute Attribute = EAttackAttribute::DamagePhysical;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	FLootFloatRange Value;
};

/** Source parameters: ranges for the numbers, fixed ammo type. */
USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FLootSourceParams
{
	GENERATED_BODY()

	FPowerSourceParams Roll(FRandomStream& Stream, float LevelMultiplier) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	FLootIntRange Power = FLootIntRange(1, 1);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (Categories = "Ammo"))
	FGameplayTag AmmoType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	FLootIntRange Capacity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	FLootIntRange CostPerUse = FLootIntRange(1, 1);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	FLootFloatRange RechargeDelay = FLootFloatRange(1.f, 1.f);
};

/** Receiver parameters: fixed ability, ranges for the numbers. */
USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FLootReceiverParams
{
	GENERATED_BODY()

	FPowerReceiverParams Roll(FRandomStream& Stream, float LevelMultiplier) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	TSubclassOf<UGameplayAbility> Ability;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	FLootIntRange RequiredPower = FLootIntRange(1, 1);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	TArray<FLootAttackStatRange> BaseStats;
};
