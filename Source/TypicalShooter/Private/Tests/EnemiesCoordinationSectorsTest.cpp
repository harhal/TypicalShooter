// Tests for EnemySectors: even spread around the target, nearest free slot, flankers outside the target's view, reaim
// hysteresis.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Enemies/Coordination/EnemyCoordinationSettings.h"
#include "Enemies/Coordination/EnemySectors.h"

namespace EnemiesCoordinationSectorsTest
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	FEnemySectorQuery Query(const TArray<TPair<EEnemyRole, float>>& Members, const float ViewYaw = 0.f)
	{
		FEnemySectorQuery Q;
		Q.ViewYaw = ViewYaw;
		Q.FlankViewHalfAngle = 60.f;
		for (const TPair<EEnemyRole, float>& Member : Members)
		{
			Q.Members.Add({Member.Key, Member.Value});
		}
		return Q;
	}

	float Gap(const float A, const float B) { return FMath::Abs(FMath::FindDeltaAngleDegrees(A, B)); }

	float MinPairGap(const TArray<float>& Yaws)
	{
		float Min = 360.f;
		for (int32 I = 0; I < Yaws.Num(); ++I)
		{
			for (int32 J = I + 1; J < Yaws.Num(); ++J)
			{
				Min = FMath::Min(Min, Gap(Yaws[I], Yaws[J]));
			}
		}
		return Min;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesCoordinationSectorsEvenTest, "TypicalShooter.Enemies.Coordination.Sectors.EvenSpread",
	EnemiesCoordinationSectorsTest::Flags)
bool FEnemiesCoordinationSectorsEvenTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesCoordinationSectorsTest;
	// A clump of four in front of the target spreads into four quarters.
	const TArray<float> Yaws = EnemySectors::Assign(Query({{EEnemyRole::Pressure, 0.f}, {EEnemyRole::Pressure, 10.f},
		{EEnemyRole::Cover, 20.f}, {EEnemyRole::Cover, 30.f}}));
	TestEqual(TEXT("One sector per enemy"), Yaws.Num(), 4);
	TestTrue(FString::Printf(TEXT("Quarters apart (min gap %.1f)"), MinPairGap(Yaws)), MinPairGap(Yaws) > 89.f);
	TestEqual(TEXT("Sector half width for 4"), EnemySectors::SectorHalfWidth(4), 45.f);
	TestEqual(TEXT("Empty group"), EnemySectors::Assign(Query({})).Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesCoordinationSectorsStableTest, "TypicalShooter.Enemies.Coordination.Sectors.NearestFree",
	EnemiesCoordinationSectorsTest::Flags)
bool FEnemiesCoordinationSectorsStableTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesCoordinationSectorsTest;
	// Already spread evenly: everyone keeps its own angle.
	const TArray<float> Current = {10.f, 100.f, -170.f, -80.f};
	const TArray<float> Kept = EnemySectors::Assign(Query({{EEnemyRole::Pressure, Current[0]}, {EEnemyRole::Cover, Current[1]},
		{EEnemyRole::Pressure, Current[2]}, {EEnemyRole::Cover, Current[3]}}));
	for (int32 Index = 0; Index < Current.Num(); ++Index)
	{
		TestTrue(FString::Printf(TEXT("Enemy %d keeps its angle (%.1f -> %.1f)"), Index, Current[Index], Kept[Index]),
			Gap(Current[Index], Kept[Index]) < 1.f);
	}

	// Almost even: nobody runs around the target, everyone takes the slot next to it.
	const TArray<float> Rough = {0.f, 80.f, 170.f, -100.f};
	const TArray<float> Moved = EnemySectors::Assign(Query({{EEnemyRole::Pressure, Rough[0]}, {EEnemyRole::Pressure, Rough[1]},
		{EEnemyRole::Cover, Rough[2]}, {EEnemyRole::Cover, Rough[3]}}));
	for (int32 Index = 0; Index < Rough.Num(); ++Index)
	{
		TestTrue(FString::Printf(TEXT("Enemy %d moves little (%.1f -> %.1f)"), Index, Rough[Index], Moved[Index]),
			Gap(Rough[Index], Moved[Index]) < 25.f);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesCoordinationSectorsFlankTest, "TypicalShooter.Enemies.Coordination.Sectors.FlankersOutOfView",
	EnemiesCoordinationSectorsTest::Flags)
bool FEnemiesCoordinationSectorsFlankTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesCoordinationSectorsTest;
	// The target looks along yaw 90; the flanker stands right in front of it.
	const TArray<float> Yaws = EnemySectors::Assign(Query({{EEnemyRole::Pressure, 80.f}, {EEnemyRole::Flank, 95.f},
		{EEnemyRole::Pressure, 0.f}, {EEnemyRole::Cover, 180.f}}, 90.f));
	TestTrue(FString::Printf(TEXT("Flanker outside the view cone (%.1f)"), Yaws[1]), Gap(Yaws[1], 90.f) > 60.f);
	TestTrue(TEXT("Still evenly spread"), MinPairGap(Yaws) > 89.f);

	// A lone flanker in front of the target goes behind it.
	const TArray<float> Lone = EnemySectors::Assign(Query({{EEnemyRole::Flank, 5.f}}, 0.f));
	TestTrue(FString::Printf(TEXT("Lone flanker goes behind (%.1f)"), Lone[0]), Gap(Lone[0], 180.f) < 1.f);

	// Two flankers among three: both out of view.
	const TArray<float> Two = EnemySectors::Assign(Query({{EEnemyRole::Flank, 0.f}, {EEnemyRole::Flank, 10.f},
		{EEnemyRole::Pressure, -10.f}}, 0.f));
	TestTrue(FString::Printf(TEXT("First flanker out of view (%.1f)"), Two[0]), Gap(Two[0], 0.f) > 60.f);
	TestTrue(FString::Printf(TEXT("Second flanker out of view (%.1f)"), Two[1]), Gap(Two[1], 0.f) > 60.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesCoordinationSectorsReaimTest, "TypicalShooter.Enemies.Coordination.Sectors.ReaimHysteresis",
	EnemiesCoordinationSectorsTest::Flags)
bool FEnemiesCoordinationSectorsReaimTest::RunTest(const FString& Parameters)
{
	TestFalse(TEXT("Turned 20 of 30: keep"), EnemySectors::ShouldReaimFlank(0.f, 20.f, 30.f));
	TestTrue(TEXT("Turned 40 of 30: reaim"), EnemySectors::ShouldReaimFlank(0.f, 40.f, 30.f));
	TestFalse(TEXT("Across the wrap: 20 degrees"), EnemySectors::ShouldReaimFlank(170.f, -170.f, 30.f));
	TestTrue(TEXT("Across the wrap: 50 degrees"), EnemySectors::ShouldReaimFlank(160.f, -150.f, 30.f));

	TestNearlyEqual(TEXT("Yaw of a point east of the target"), EnemySectors::YawAround(FVector(100.f, 0.f, 0.f), FVector(300.f, 0.f, 50.f)), 0.f);
	TestNearlyEqual(TEXT("Yaw of a point north of the target"), EnemySectors::YawAround(FVector::ZeroVector, FVector(0.f, 500.f, 0.f)), 90.f);

	const FEnemySectorSettings& Project = GetDefault<UEnemyCoordinationSettings>()->Sectors;
	TestEqual(TEXT("Default flank view half angle"), Project.FlankViewHalfAngle, 60.f);
	TestEqual(TEXT("Default flank reaim"), Project.FlankReaimDegrees, 30.f);
	return true;
}

#endif
