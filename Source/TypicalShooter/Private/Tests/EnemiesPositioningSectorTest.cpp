// Tests for EnemyPositioning with a sector from the coordinator: stand at the sector's yaw, jitter within it, keep
// neighbouring stand points apart.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Enemies/EnemyPositioning.h"
#include "Enemies/Coordination/EnemySectors.h"

namespace EnemiesPositioningSectorTest
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	FEnemyStandQuery Query(const float SelfYaw, const float SelfDistance, const float SectorYaw, const float HalfWidth,
		const bool bLineOfSight = true)
	{
		FEnemyStandQuery Q;
		Q.Target = FVector(100.f, -200.f, 50.f);
		Q.Self = Q.Target + FVector(SelfDistance, 0.f, 0.f).RotateAngleAxis(SelfYaw, FVector::UpVector);
		Q.Self.Z = 100.f;
		Q.MinDistance = 800.f;
		Q.MaxDistance = 1600.f;
		Q.JitterDegrees = 20.f;
		Q.FlankDegrees = 60.f;
		Q.bHasLineOfSight = bLineOfSight;
		Q.SectorYaw = SectorYaw;
		Q.SectorHalfWidth = HalfWidth;
		Q.MinSeparation = 250.f;
		return Q;
	}

	float YawGap(const FEnemyStandQuery& Q, const FVector& Point, const float Yaw)
	{
		return FMath::Abs(FMath::FindDeltaAngleDegrees(EnemySectors::YawAround(Q.Target, Point), Yaw));
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesPositioningSectorMoveTest, "TypicalShooter.Enemies.Positioning.SectorAngleMovesIntoSector",
	EnemiesPositioningSectorTest::Flags)
bool FEnemiesPositioningSectorMoveTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesPositioningSectorTest;
	for (int32 Seed = 0; Seed < 20; ++Seed)
	{
		FRandomStream Random(Seed);
		// In the band and visible, but a quarter turn away from its sector.
		const FEnemyStandQuery Q = Query(0.f, 1200.f, 90.f, 45.f);
		FVector Point;
		TestTrue(TEXT("Outside its sector: move"), EnemyPositioning::ChooseStandPoint(Q, Random, Point));
		TestTrue(FString::Printf(TEXT("Lands in the sector (gap %.1f)"), YawGap(Q, Point, 90.f)), YawGap(Q, Point, 90.f) <= 45.f);
		TestEqual(TEXT("Middle of the band"), float(FVector::Dist2D(Point, Q.Target)), 1200.f, 1.f);
		TestEqual(TEXT("Own height"), Point.Z, 100.0);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesPositioningSectorStayTest, "TypicalShooter.Enemies.Positioning.SectorAngleStaysInside",
	EnemiesPositioningSectorTest::Flags)
bool FEnemiesPositioningSectorStayTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesPositioningSectorTest;
	FRandomStream Random(1);
	FVector Point;
	TestFalse(TEXT("In the band, visible, inside the sector: stay"),
		EnemyPositioning::ChooseStandPoint(Query(110.f, 1000.f, 90.f, 45.f), Random, Point));
	TestTrue(TEXT("Too close even inside the sector: move"),
		EnemyPositioning::ChooseStandPoint(Query(90.f, 300.f, 90.f, 45.f), Random, Point));

	// No line of sight: near edge of the band, still within the sector.
	for (int32 Seed = 0; Seed < 20; ++Seed)
	{
		FRandomStream SeedRandom(Seed);
		const FEnemyStandQuery Q = Query(90.f, 1200.f, 90.f, 30.f, false);
		TestTrue(TEXT("No sight: move"), EnemyPositioning::ChooseStandPoint(Q, SeedRandom, Point));
		TestEqual(TEXT("No sight: near edge"), float(FVector::Dist2D(Point, Q.Target)), 800.f, 1.f);
		TestTrue(TEXT("No sight: flank turn limited to the sector"), YawGap(Q, Point, 90.f) <= 30.f);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesPositioningSeparationTest, "TypicalShooter.Enemies.Positioning.Separation",
	EnemiesPositioningSectorTest::Flags)
bool FEnemiesPositioningSeparationTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesPositioningSectorTest;
	for (const int32 GroupSize : {4, 6, 12})
	{
		const float HalfWidth = EnemySectors::SectorHalfWidth(GroupSize);
		for (int32 Seed = 0; Seed < 30; ++Seed)
		{
			FRandomStream Random(Seed);
			TArray<FVector> Points;
			for (int32 Member = 0; Member < GroupSize; ++Member)
			{
				// Everyone starts on top of the target and gets its own evenly spaced sector.
				const FEnemyStandQuery Q = Query(0.f, 0.f, Member * 360.f / GroupSize, HalfWidth);
				FVector Point;
				EnemyPositioning::ChooseStandPoint(Q, Random, Point);
				Points.Add(Point);
			}
			float Closest = UE_BIG_NUMBER;
			for (int32 I = 0; I < Points.Num(); ++I)
			{
				for (int32 J = I + 1; J < Points.Num(); ++J)
				{
					Closest = FMath::Min(Closest, float(FVector::Dist2D(Points[I], Points[J])));
				}
			}
			TestTrue(FString::Printf(TEXT("%d enemies, seed %d: closest pair %.0f >= 250"), GroupSize, Seed, Closest), Closest >= 249.f);
		}
	}
	return true;
}

#endif
