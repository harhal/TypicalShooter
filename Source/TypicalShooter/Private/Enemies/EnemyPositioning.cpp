#include "Enemies/EnemyPositioning.h"

namespace EnemyPositioning::Private
{
	/** Horizontal direction from the target to the enemy; a random one when they stand on the same spot. */
	FVector AwayFromTarget(const FEnemyStandQuery& Query, FRandomStream& Random)
	{
		const FVector Away = (Query.Self - Query.Target).GetSafeNormal2D();
		return Away.IsNearlyZero() ? FVector(1.f, 0.f, 0.f).RotateAngleAxis(Random.FRandRange(0.f, 360.f), FVector::UpVector) : Away;
	}

	/** The sector's direction when the coordinator gave one, otherwise away from the target. */
	FVector BaseDirection(const FEnemyStandQuery& Query, FRandomStream& Random)
	{
		return Query.SectorYaw.IsSet()
			? FVector(1.f, 0.f, 0.f).RotateAngleAxis(Query.SectorYaw.GetValue(), FVector::UpVector)
			: AwayFromTarget(Query, Random);
	}

	bool IsInsideSector(const FEnemyStandQuery& Query)
	{
		if (!Query.SectorYaw.IsSet())
		{
			return true;
		}
		const FVector Offset = Query.Self - Query.Target;
		const float Yaw = FMath::RadiansToDegrees(FMath::Atan2(Offset.Y, Offset.X));
		return FMath::Abs(FMath::FindDeltaAngleDegrees(Yaw, Query.SectorYaw.GetValue())) <= Query.SectorHalfWidth;
	}

	bool ShouldStay(const FEnemyStandQuery& Query)
	{
		const float Distance = FVector::Dist2D(Query.Self, Query.Target);
		return Query.bHasLineOfSight && Distance >= Query.MinDistance && Distance <= Query.MaxDistance && IsInsideSector(Query);
	}

	float StandRadius(const FEnemyStandQuery& Query)
	{
		return Query.bHasLineOfSight ? 0.5f * (Query.MinDistance + Query.MaxDistance) : Query.MinDistance;
	}

	/** Point at the stand radius from the target in Direction turned by Degrees, at the enemy's height. */
	FVector PointAt(const FEnemyStandQuery& Query, const FVector& Direction, const float Degrees)
	{
		FVector Point = Query.Target + Direction.RotateAngleAxis(Degrees, FVector::UpVector) * StandRadius(Query);
		Point.Z = Query.Self.Z;
		return Point;
	}

	/**
	 * Within a sector the turn stays inside it, minus half the angle MinSeparation spans at the stand radius: points of
	 * two neighbouring sectors then never come closer than MinSeparation.
	 */
	float SectorTurnLimit(const FEnemyStandQuery& Query)
	{
		const float Radius = FMath::Max(StandRadius(Query), 1.f);
		const float SeparationHalfAngle = FMath::RadiansToDegrees(FMath::Asin(FMath::Clamp(Query.MinSeparation / (2.f * Radius), 0.f, 1.f)));
		return FMath::Max(0.f, Query.SectorHalfWidth - SeparationHalfAngle);
	}

	float RandomTurn(const FEnemyStandQuery& Query, FRandomStream& Random)
	{
		float MaxTurn = Query.bHasLineOfSight ? Query.JitterDegrees : Query.FlankDegrees;
		if (Query.SectorYaw.IsSet())
		{
			MaxTurn = FMath::Min(MaxTurn, SectorTurnLimit(Query));
		}
		return Random.FRandRange(-MaxTurn, MaxTurn);
	}
}

bool EnemyPositioning::ChooseStandPoint(const FEnemyStandQuery& Query, FRandomStream& Random, FVector& OutPoint)
{
	if (Private::ShouldStay(Query))
	{
		return false;
	}
	const FVector Away = Private::BaseDirection(Query, Random);
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
	const FVector Away = Private::BaseDirection(Query, Random);
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
