#include "Enemies/EnemyPositioning.h"

namespace EnemyPositioning::Private
{
	/** Horizontal direction from the target to the enemy; a random one when they stand on the same spot. */
	FVector AwayFromTarget(const FEnemyStandQuery& Query, FRandomStream& Random)
	{
		const FVector Away = (Query.Self - Query.Target).GetSafeNormal2D();
		return Away.IsNearlyZero() ? FVector(1.f, 0.f, 0.f).RotateAngleAxis(Random.FRandRange(0.f, 360.f), FVector::UpVector) : Away;
	}

	bool ShouldStay(const FEnemyStandQuery& Query)
	{
		const float Distance = FVector::Dist2D(Query.Self, Query.Target);
		return Query.bHasLineOfSight && Distance >= Query.MinDistance && Distance <= Query.MaxDistance;
	}

	/** Point at Radius from the target in Direction turned by Degrees, at the enemy's height. */
	FVector PointAt(const FEnemyStandQuery& Query, const FVector& Direction, const float Degrees)
	{
		const float Radius = Query.bHasLineOfSight ? 0.5f * (Query.MinDistance + Query.MaxDistance) : Query.MinDistance;
		FVector Point = Query.Target + Direction.RotateAngleAxis(Degrees, FVector::UpVector) * Radius;
		Point.Z = Query.Self.Z;
		return Point;
	}

	float RandomTurn(const FEnemyStandQuery& Query, FRandomStream& Random)
	{
		const float MaxTurn = Query.bHasLineOfSight ? Query.JitterDegrees : Query.FlankDegrees;
		return Random.FRandRange(-MaxTurn, MaxTurn);
	}
}

bool EnemyPositioning::ChooseStandPoint(const FEnemyStandQuery& Query, FRandomStream& Random, FVector& OutPoint)
{
	if (Private::ShouldStay(Query))
	{
		return false;
	}
	const FVector Away = Private::AwayFromTarget(Query, Random);
	OutPoint = Private::PointAt(Query, Away, Private::RandomTurn(Query, Random));
	return true;
}

bool EnemyPositioning::ChooseReachableStandPoint(const FEnemyStandQuery& Query, FRandomStream& Random,
	TFunctionRef<bool(FVector&)> Accept, FVector& OutPoint)
{
	if (Private::ShouldStay(Query))
	{
		return false;
	}
	const FVector Away = Private::AwayFromTarget(Query, Random);
	const float Turn = Private::RandomTurn(Query, Random);
	// The preferred point first, then further and further around the target, alternating sides.
	static constexpr float Sweep[] = { 0.f, 30.f, -30.f, 60.f, -60.f, 90.f, -90.f, 120.f, -120.f, 150.f, -150.f, 180.f };
	for (const float Extra : Sweep)
	{
		FVector Point = Private::PointAt(Query, Away, Turn + Extra);
		if (Accept(Point))
		{
			OutPoint = Point;
			return true;
		}
	}
	return false;
}
