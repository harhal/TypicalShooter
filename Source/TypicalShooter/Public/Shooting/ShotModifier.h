// A value for one attack parameter, independent of the slot: receiver base stats (power grid R5a) and item attack
// bonuses (R14a) are lists of these.

#pragma once

#include "CoreMinimal.h"
#include "Shooting/AttackTypes.h"
#include "ShotModifier.generated.h"

/** Value for one attack parameter. */
USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FAttackStatBonus
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot")
	EAttackAttribute Attribute = EAttackAttribute::DamagePhysical;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot")
	float Value = 0.f;
};
