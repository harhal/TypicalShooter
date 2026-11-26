// Where an enemy should stand relative to its target: keep within [MinDistance, MaxDistance], come closer when the
// target is not visible. Pure math; the AI projects the point to the navmesh and moves there.

#pragma once

#include "CoreMinimal.h"
#include "Math/RandomStream.h"

struct FEnemyStandQuery
{
	FVector Self = FVector::ZeroVector;
	FVector Target = FVector::ZeroVector;
	float MinDistance = 800.f;
	float MaxDistance = 1600.f;
	/** Random turn of the stand point around the target, so enemies do not line up. */
	float JitterDegrees = 20.f;
	/** Random turn around the target when approaching without line of sight (to find another angle). */
	float FlankDegrees = 60.f;
	bool bHasLineOfSight = true;
};

namespace EnemyPositioning
{
	/**
	 * false: stay (horizontally within the band and the target is visible).
	 * true: OutPoint = where to go: the middle of the band (or its near edge without line of sight), on the enemy's side of
	 * the target turned by a random angle within the jitter (flank angle without line of sight), at the enemy's own height.
	 */
	TYPICALSHOOTER_API bool ChooseStandPoint(const FEnemyStandQuery& Query, FRandomStream& Random, FVector& OutPoint);

	/**
	 * ChooseStandPoint, but the point must pass Accept (which may adjust it, e.g. project it to the navmesh). Rejected points
	 * are retried turned further around the target (up to the opposite side). false: stay, or nothing acceptable.
	 */
	TYPICALSHOOTER_API bool ChooseReachableStandPoint(const FEnemyStandQuery& Query, FRandomStream& Random,
		TFunctionRef<bool(FVector&)> Accept, FVector& OutPoint);
}
