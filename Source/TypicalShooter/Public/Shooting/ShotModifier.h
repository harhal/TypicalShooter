// Per-shot modifiers that cost reserve ammo (R15a): paid -> the shot gets the bonuses, unpaid -> skipped.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Shooting/AttackTypes.h"
#include "ShotModifier.generated.h"

/** Adds Value to one parameter of a single shot. */
USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FAttackStatBonus
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot")
	EAttackAttribute Attribute = EAttackAttribute::DamagePhysical;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot")
	float Value = 0.f;
};

USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FAmmoGatedShotModifier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot")
	EAttackSlot Slot = EAttackSlot::Primary;

	/** Reserve ammo kind paid per shot (Ammo.*). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot", meta = (Categories = "Ammo"))
	FGameplayTag AmmoType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot", meta = (ClampMin = 1))
	int32 CostPerShot = 1;

	/** Shot parameters changed while paid (damage, gravity, damage loss, speed, distances, pattern scale). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot")
	TArray<FAttackStatBonus> Bonuses;
};
