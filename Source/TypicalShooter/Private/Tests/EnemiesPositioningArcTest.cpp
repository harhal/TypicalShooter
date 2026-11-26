// Tests for EnemyPositioning::NextArcWaypoint: an enemy whose stand point is on the other side of the target goes around
// it along an arc within its distance band instead of running through the target.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Enemies/Coordination/EnemySectors.h"
#include "Enemies/EnemyPositioning.h"

namespace EnemiesPositioningArcTest
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	const FVector Target(100.f, 200.f, 0.f);

	FVector At(const float Yaw, const float Distance, const float Z = 90.f)
	{
		FVector Point = Target + FVector(Distance, 0.f, 0.f).RotateAngleAxis(Yaw, FVector::UpVector);
		Point.Z = Z;
		return Point;
	}

	FEnemyArcQuery Query(const FVector& Self, const FVector& Goal)
	{
		FEnemyArcQuery Q;
		Q.Self = Self;
		Q.Target = Target;
		Q.Goal = Goal;
		Q.MinDistance = 500.f;
		Q.MaxDistance = 900.f;
		Q.MaxStepDegrees = 45.f;
		return Q;
	}

	float YawOf(const FVector& Point) { return EnemySectors::YawAround(Target, Point); }
	float Gap(const float A, const float B) { return FMath::Abs(FMath::FindDeltaAngleDegrees(A, B)); }

	/** Closest horizontal distance from the target to the segment A-B. */
	float SegmentDistance(const FVector& A, const FVector& B)
	{
		const FVector Closest = FMath::ClosestPointOnSegment(FVector(Target.X, Target.Y, 0.f), FVector(A.X, A.Y, 0.f), FVector(B.X, B.Y, 0.f));
		return FVector::Dist2D(Closest, Target);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesArcNearTest, "TypicalShooter.Enemies.Positioning.ArcGoesStraightWhenClose",
	EnemiesPositioningArcTest::Flags)
bool FEnemiesArcNearTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesPositioningArcTest;
	const FVector Goal = At(30.f, 700.f);
	TestEqual(TEXT("Goal within one step around the target: go straight"),
		EnemyPositioning::NextArcWaypoint(Query(At(0.f, 800.f), Goal)), Goal);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesArcAroundTest, "TypicalShooter.Enemies.Positioning.ArcGoesAroundTarget",
	EnemiesPositioningArcTest::Flags)
bool FEnemiesArcAroundTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesPositioningArcTest;
	// Behind the target, 1100 away (beyond the band); the goal is right in front of it.
	FVector Self = At(180.f, 1100.f);
	const FVector Goal = At(0.f, 700.f);
	const FVector First = EnemyPositioning::NextArcWaypoint(Query(Self, Goal));
	TestEqual(TEXT("One step around the target"), Gap(YawOf(First), 180.f), 45.f, 0.5f);
	TestEqual(TEXT("At the band distance (current distance clamped to the band)"), float(FVector::Dist2D(First, Target)), 900.f, 1.f);
	TestEqual(TEXT("At the enemy's own height"), First.Z, Self.Z);

	// Walk the waypoints: never through the inner edge of the band, and arrive at the goal.
	float Closest = UE_BIG_NUMBER;
	int32 Steps = 0;
	for (; Steps < 10 && !Self.Equals(Goal, 1.f); ++Steps)
	{
		const FVector Next = EnemyPositioning::NextArcWaypoint(Query(Self, Goal));
		Closest = FMath::Min(Closest, SegmentDistance(Self, Next));
		Self = Next;
	}
	TestTrue(TEXT("Reaches the goal"), Self.Equals(Goal, 1.f));
	TestTrue(FString::Printf(TEXT("Stays out of the band's inner edge on the way (closest %.0f)"), Closest), Closest >= 0.85f * 500.f);
	TestTrue(FString::Printf(TEXT("Around in a few steps (%d)"), Steps), Steps <= 5);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesArcShorterSideTest, "TypicalShooter.Enemies.Positioning.ArcTakesShorterSide",
	EnemiesPositioningArcTest::Flags)
bool FEnemiesArcShorterSideTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesPositioningArcTest;
	const FVector Waypoint = EnemyPositioning::NextArcWaypoint(Query(At(170.f, 700.f), At(-100.f, 700.f)));
	TestEqual(TEXT("170 -> -100 is +90 the short way: first step to 215 (= -145)"), Gap(YawOf(Waypoint), -145.f), 0.f, 0.5f);

	const FVector Inside = EnemyPositioning::NextArcWaypoint(Query(At(0.f, 200.f), At(180.f, 700.f)));
	TestEqual(TEXT("Too close: the arc runs at the band's inner edge"), float(FVector::Dist2D(Inside, Target)), 500.f, 1.f);
	return true;
}

#endif
