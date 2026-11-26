// Tests for EnemyRoleStand::Apply: how a role and the attack token shape the stand query (R10).

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Enemies/Coordination/EnemyCoordinationSettings.h"
#include "Enemies/Coordination/EnemyRoleStand.h"

namespace EnemiesCoordinationRoleStandTest
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	EnemyRoleStand::FInput Input(const EEnemyRole Role, const bool bHasToken)
	{
		EnemyRoleStand::FInput In;
		In.Role = Role;
		In.bHasToken = bHasToken;
		In.Band = {600.f, 1000.f};
		In.SectorYaw = 90.f;
		In.SectorHalfWidth = 45.f;
		In.MinSeparation = 250.f;
		return In;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesRoleStandPressureTest, "TypicalShooter.Enemies.Coordination.RoleStand.PressureClosesInWithoutToken",
	EnemiesCoordinationRoleStandTest::Flags)
bool FEnemiesRoleStandPressureTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesCoordinationRoleStandTest;
	FEnemyStandQuery Waiting;
	EnemyRoleStand::Apply(Waiting, Input(EEnemyRole::Pressure, false));
	TestEqual(TEXT("Without token: near half - min"), Waiting.MinDistance, 600.f);
	TestEqual(TEXT("Without token: near half - max"), Waiting.MaxDistance, 800.f);

	FEnemyStandQuery Attacking;
	EnemyRoleStand::Apply(Attacking, Input(EEnemyRole::Pressure, true));
	TestEqual(TEXT("With token: whole band - min"), Attacking.MinDistance, 600.f);
	TestEqual(TEXT("With token: whole band - max"), Attacking.MaxDistance, 1000.f);
	TestTrue(TEXT("Sector applied"), Attacking.SectorYaw.IsSet() && Attacking.SectorYaw.GetValue() == 90.f);
	TestEqual(TEXT("Sector width applied"), Attacking.SectorHalfWidth, 45.f);
	TestEqual(TEXT("Separation applied"), Attacking.MinSeparation, 250.f);
	TestTrue(TEXT("Pressure keeps to its sector"), Attacking.bSectorRequiredToStay);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesRoleStandCoverTest, "TypicalShooter.Enemies.Coordination.RoleStand.CoverHoldsWhileVisible",
	EnemiesCoordinationRoleStandTest::Flags)
bool FEnemiesRoleStandCoverTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesCoordinationRoleStandTest;
	FEnemyStandQuery Waiting;
	EnemyRoleStand::Apply(Waiting, Input(EEnemyRole::Cover, false));
	TestFalse(TEXT("Waiting cover holds anywhere in its band"), Waiting.bSectorRequiredToStay);
	TestEqual(TEXT("Whole band"), Waiting.MaxDistance, 1000.f);
	TestTrue(TEXT("Still heads for its sector when it must move"), Waiting.SectorYaw.IsSet());

	// Inside the band and visible, but outside its sector: the waiting cover enemy stays put.
	Waiting.Target = FVector::ZeroVector;
	Waiting.Self = FVector(800.f, 0.f, 0.f); // yaw 0, sector is 90 +- 45
	Waiting.bHasLineOfSight = true;
	FRandomStream Random(1);
	FVector Point;
	TestFalse(TEXT("Holds position"), EnemyPositioning::ChooseStandPoint(Waiting, Random, Point));

	FEnemyStandQuery Attacking;
	EnemyRoleStand::Apply(Attacking, Input(EEnemyRole::Cover, true));
	TestTrue(TEXT("With token it takes its sector"), Attacking.bSectorRequiredToStay);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesRoleStandFlankTest, "TypicalShooter.Enemies.Coordination.RoleStand.FlankGoesToSector",
	EnemiesCoordinationRoleStandTest::Flags)
bool FEnemiesRoleStandFlankTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesCoordinationRoleStandTest;
	for (const bool bToken : {false, true})
	{
		FEnemyStandQuery Query;
		EnemyRoleStand::Apply(Query, Input(EEnemyRole::Flank, bToken));
		TestTrue(TEXT("Flanker keeps to its sector"), Query.bSectorRequiredToStay);
		TestEqual(TEXT("Flanker: whole band"), Query.MaxDistance, 1000.f);
	}

	const FEnemyRoleBands& Bands = GetDefault<UEnemyCoordinationSettings>()->RoleBands;
	TestEqual(TEXT("Default pressure band"), Bands.Get(EEnemyRole::Pressure).Max, 900.f);
	TestEqual(TEXT("Default flank band"), Bands.Get(EEnemyRole::Flank).Min, 700.f);
	TestEqual(TEXT("Default cover band"), Bands.Get(EEnemyRole::Cover).Max, 2000.f);
	return true;
}

#endif
