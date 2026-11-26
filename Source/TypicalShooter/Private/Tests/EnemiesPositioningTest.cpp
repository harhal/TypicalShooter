// Tests for EnemyPositioning::ChooseStandPoint: keep within [Min, Max] of the target, come closer without line of sight.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Enemies/EnemyPositioning.h"

namespace EnemiesPositioningTest
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	FEnemyStandQuery Query(const FVector& Self, const bool bLineOfSight = true)
	{
		FEnemyStandQuery Q;
		Q.Self = Self;
		Q.Target = FVector(0.f, 0.f, 50.f);
		Q.MinDistance = 800.f;
		Q.MaxDistance = 1600.f;
		Q.JitterDegrees = 20.f;
		Q.bHasLineOfSight = bLineOfSight;
		return Q;
	}

	float Distance2D(const FVector& A, const FVector& B) { return FVector::Dist2D(A, B); }

	float AngleDegrees(const FVector& A, const FVector& B)
	{
		return FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(A.GetSafeNormal2D() | B.GetSafeNormal2D(), -1.f, 1.f)));
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesPositioningInsideTest, "TypicalShooter.Enemies.Positioning.InsideBandStays", EnemiesPositioningTest::Flags)
bool FEnemiesPositioningInsideTest::RunTest(const FString& Parameters)
{
	FRandomStream Random(1);
	const FEnemyStandQuery Q = EnemiesPositioningTest::Query(FVector(1200.f, 0.f, 100.f));
	FVector Point;
	TestFalse(TEXT("Inside the band: no need to move"), EnemyPositioning::ChooseStandPoint(Q, Random, Point));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesPositioningCloseTest, "TypicalShooter.Enemies.Positioning.TooCloseBacksOff", EnemiesPositioningTest::Flags)
bool FEnemiesPositioningCloseTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesPositioningTest;
	FRandomStream Random(2);
	const FEnemyStandQuery Q = Query(FVector(0.f, 300.f, 100.f));
	FVector Point;
	TestTrue(TEXT("Too close: move"), EnemyPositioning::ChooseStandPoint(Q, Random, Point));
	TestEqual(TEXT("To the middle of the band"), Distance2D(Point, Q.Target), 1200.f, 1.f);
	TestTrue(TEXT("Away on the same side (within jitter)"), AngleDegrees(Point - Q.Target, Q.Self - Q.Target) <= Q.JitterDegrees + 0.1f);
	TestEqual(TEXT("Keeps own height"), Point.Z, 100.0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesPositioningFarTest, "TypicalShooter.Enemies.Positioning.TooFarApproaches", EnemiesPositioningTest::Flags)
bool FEnemiesPositioningFarTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesPositioningTest;
	for (int32 Seed = 0; Seed < 20; ++Seed)
	{
		FRandomStream Random(Seed);
		const FEnemyStandQuery Q = Query(FVector(-4000.f, 1000.f, 100.f));
		FVector Point;
		TestTrue(TEXT("Too far: move"), EnemyPositioning::ChooseStandPoint(Q, Random, Point));
		TestEqual(TEXT("To the middle of the band"), Distance2D(Point, Q.Target), 1200.f, 1.f);
		TestTrue(TEXT("Random offset stays within the jitter"), AngleDegrees(Point - Q.Target, Q.Self - Q.Target) <= Q.JitterDegrees + 0.1f);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesPositioningNoSightTest, "TypicalShooter.Enemies.Positioning.NoLineOfSightApproaches", EnemiesPositioningTest::Flags)
bool FEnemiesPositioningNoSightTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesPositioningTest;
	FRandomStream Random(3);
	const FEnemyStandQuery Q = Query(FVector(1200.f, 0.f, 100.f), false);
	FVector Point;
	TestTrue(TEXT("No line of sight: move even inside the band"), EnemyPositioning::ChooseStandPoint(Q, Random, Point));
	TestEqual(TEXT("To the near edge of the band"), Distance2D(Point, Q.Target), 800.f, 1.f);
	TestTrue(TEXT("Flanks within the flank angle"), AngleDegrees(Point - Q.Target, Q.Self - Q.Target) <= Q.FlankDegrees + 0.1f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesPositioningSameSpotTest, "TypicalShooter.Enemies.Positioning.OnTopOfTarget", EnemiesPositioningTest::Flags)
bool FEnemiesPositioningSameSpotTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesPositioningTest;
	FRandomStream Random(4);
	const FEnemyStandQuery Q = Query(FVector(0.f, 0.f, 100.f));
	FVector Point;
	TestTrue(TEXT("On top of the target: move"), EnemyPositioning::ChooseStandPoint(Q, Random, Point));
	TestEqual(TEXT("Some direction, middle of the band"), Distance2D(Point, Q.Target), 1200.f, 1.f);
	return true;
}

#endif
