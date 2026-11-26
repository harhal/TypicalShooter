// Roles of an enemy in its target group. The role decides the distance band, the sector and what the enemy does while
// it waits for an attack token; what "attacking" means (shooting, melee, a kamikaze rush) is up to the enemy itself.

#pragma once

#include "CoreMinimal.h"
#include "EnemyRole.generated.h"

UENUM(BlueprintType)
enum class EEnemyRole : uint8
{
	/** Close band; closes in while waiting; first in line for tokens. */
	Pressure,
	/** Comes from outside the target's view (side / back). */
	Flank,
	/** Far band; holds position while it sees the target. */
	Cover,
};

/** Wanted share of each role in a group (relative weights). */
USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FEnemyRoleWeights
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Roles", meta = (ClampMin = "0"))
	float Pressure = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Roles", meta = (ClampMin = "0"))
	float Flank = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Roles", meta = (ClampMin = "0"))
	float Cover = 1.f;

	float Get(EEnemyRole Role) const;
};
