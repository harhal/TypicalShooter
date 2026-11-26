#include "Enemies/EnemyPositioning.h"

namespace EnemyPositioning::Private
{
	/** Horizontal direction from the target to the enemy; a random one when they stand on the same spot. */
	FVector AwayFromTarget(const FEnemyStandQuery& Query, FRandomStream& Random)
	{
		const FVector Away = (Query.Self - Query.Target).GetSafeNormal2D();
		return Away.IsNearlyZero() ? FVector(1.f, 0.f, 0.f).RotateAngleAxis(Random.FRandRange(0.f, 360.f), FVector::UpVector) : Away;
	}
}

bool EnemyPositioning::ChooseStandPoint(const FEnemyStandQuery& Query, FRandomStream& Random, FVector& OutPoint)
{
	const float Distance = FVector::Dist2D(Query.Self, Query.Target);
	const bool bInsideBand = Distance >= Query.MinDistance && Distance <= Query.MaxDistance;
	if (bInsideBand && Query.bHasLineOfSight)
	{
		return false;
	}

	const float Radius = Query.bHasLineOfSight ? 0.5f * (Query.MinDistance + Query.MaxDistance) : Query.MinDistance;
	const float MaxTurn = Query.bHasLineOfSight ? Query.JitterDegrees : Query.FlankDegrees;
	const FVector Direction = Private::AwayFromTarget(Query, Random).RotateAngleAxis(Random.FRandRange(-MaxTurn, MaxTurn), FVector::UpVector);

	OutPoint = Query.Target + Direction * Radius;
	OutPoint.Z = Query.Self.Z;
	return true;
}
