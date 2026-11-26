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
	/** Yaw around the target the coordinator gave this enemy; unset = stand on the enemy's own side. */
	TOptional<float> SectorYaw;
	/** Half width of that sector: the enemy stays while inside it, and the stand point jitters within it. */
	float SectorHalfWidth = 180.f;
	/** false: the enemy may stay anywhere in the band (the sector only steers where it goes when it has to move). */
	bool bSectorRequiredToStay = true;
	/** Neighbouring sectors' stand points keep at least this far apart (jitter is narrowed for it). */
	float MinSeparation = 0.f;
};

/** Moving to Goal around Target without crossing it. */
struct FEnemyArcQuery
{
	FVector Self = FVector::ZeroVector;
	FVector Target = FVector::ZeroVector;
	FVector Goal = FVector::ZeroVector;
	/** Distance band the arc keeps to: the enemy's current distance, clamped into it. */
	float MinDistance = 800.f;
	float MaxDistance = 1600.f;
	/** Largest turn around the target per waypoint. */
	float MaxStepDegrees = 45.f;
};

namespace EnemyPositioning
{
	/**
	 * Where to walk next on the way to Goal. Goal itself when it is within MaxStepDegrees around the target (as seen from
	 * the target); otherwise a waypoint MaxStepDegrees further around the target the shorter way, at the enemy's current
	 * distance clamped into the band and at its own height - so the enemy circles the target instead of crossing it.
	 */
	TYPICALSHOOTER_API FVector NextArcWaypoint(const FEnemyArcQuery& Query);

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
