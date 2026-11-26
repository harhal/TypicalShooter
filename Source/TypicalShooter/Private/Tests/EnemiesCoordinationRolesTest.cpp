// Tests for EnemyRoles: quotas from weights, only allowed roles, stable and minimal reassignment.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Algo/Count.h"
#include "Enemies/Coordination/EnemyCoordinationSettings.h"
#include "Enemies/Coordination/EnemyRoleAssignment.h"

namespace EnemiesCoordinationRolesTest
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	using FCandidate = EnemyRoles::FCandidate;

	FEnemyRoleWeights DefaultWeights()
	{
		return FEnemyRoleWeights(); // 2 : 1 : 1
	}

	FCandidate Any() { return FCandidate(); }

	FString QuotaText(const EnemyRoles::FQuotas& Q) { return FString::Printf(TEXT("%d/%d/%d"), Q[0], Q[1], Q[2]); }

	FCandidate Holding(const EEnemyRole Role, const uint8 Allowed = EnemyRoles::AllRoles)
	{
		FCandidate C;
		C.Allowed = Allowed;
		C.Current = Role;
		return C;
	}

	FCandidate Only(const uint8 Allowed)
	{
		FCandidate C;
		C.Allowed = Allowed;
		return C;
	}

	int32 CountOf(const TArray<EEnemyRole>& Roles, const EEnemyRole Role)
	{
		return Algo::Count(Roles, Role);
	}

	int32 Changed(const TArray<FCandidate>& Before, const TArray<EEnemyRole>& After)
	{
		int32 Count = 0;
		for (int32 Index = 0; Index < Before.Num(); ++Index)
		{
			Count += Before[Index].Current.IsSet() && Before[Index].Current.GetValue() != After[Index] ? 1 : 0;
		}
		return Count;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesCoordinationRolesQuotasTest, "TypicalShooter.Enemies.Coordination.Roles.Quotas",
	EnemiesCoordinationRolesTest::Flags)
bool FEnemiesCoordinationRolesQuotasTest::RunTest(const FString& Parameters)
{
	const FEnemyRoleWeights Weights = EnemiesCoordinationRolesTest::DefaultWeights();
	TestEqual(TEXT("1 enemy -> pressure"), EnemiesCoordinationRolesTest::QuotaText(EnemyRoles::ComputeQuotas(1, Weights)), FString(TEXT("1/0/0")));
	TestEqual(TEXT("3 enemies -> one of each"), EnemiesCoordinationRolesTest::QuotaText(EnemyRoles::ComputeQuotas(3, Weights)), FString(TEXT("1/1/1")));
	TestEqual(TEXT("4 enemies -> 2 : 1 : 1"), EnemiesCoordinationRolesTest::QuotaText(EnemyRoles::ComputeQuotas(4, Weights)), FString(TEXT("2/1/1")));
	TestEqual(TEXT("8 enemies -> 4 : 2 : 2"), EnemiesCoordinationRolesTest::QuotaText(EnemyRoles::ComputeQuotas(8, Weights)), FString(TEXT("4/2/2")));

	FEnemyRoleWeights NoCover;
	NoCover.Cover = 0.f;
	TestEqual(TEXT("Zero weight -> no quota"), EnemiesCoordinationRolesTest::QuotaText(EnemyRoles::ComputeQuotas(3, NoCover)), FString(TEXT("2/1/0")));

	FEnemyRoleWeights AllZero;
	AllZero.Pressure = AllZero.Flank = AllZero.Cover = 0.f;
	TestEqual(TEXT("All weights zero -> everyone presses"), EnemiesCoordinationRolesTest::QuotaText(EnemyRoles::ComputeQuotas(2, AllZero)), FString(TEXT("2/0/0")));

	const FEnemyRoleWeights& Project = GetDefault<UEnemyCoordinationSettings>()->RoleWeights;
	TestEqual(TEXT("Project default weights 2 : 1 : 1"), EnemiesCoordinationRolesTest::QuotaText(EnemyRoles::ComputeQuotas(4, Project)), FString(TEXT("2/1/1")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesCoordinationRolesWeightsTest, "TypicalShooter.Enemies.Coordination.Roles.RespectsWeights",
	EnemiesCoordinationRolesTest::Flags)
bool FEnemiesCoordinationRolesWeightsTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesCoordinationRolesTest;
	const TArray<FCandidate> Group = {Any(), Any(), Any(), Any()};
	const TArray<EEnemyRole> Roles = EnemyRoles::Assign(Group, DefaultWeights());
	TestEqual(TEXT("One role per enemy"), Roles.Num(), 4);
	TestEqual(TEXT("Pressure x2"), CountOf(Roles, EEnemyRole::Pressure), 2);
	TestEqual(TEXT("Flank x1"), CountOf(Roles, EEnemyRole::Flank), 1);
	TestEqual(TEXT("Cover x1"), CountOf(Roles, EEnemyRole::Cover), 1);
	TestEqual(TEXT("Empty group"), EnemyRoles::Assign({}, DefaultWeights()).Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesCoordinationRolesAllowedTest, "TypicalShooter.Enemies.Coordination.Roles.OnlyAllowed",
	EnemiesCoordinationRolesTest::Flags)
bool FEnemiesCoordinationRolesAllowedTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesCoordinationRolesTest;
	const uint8 PressureOnly = EnemyRoles::Mask(EEnemyRole::Pressure);
	const uint8 CloseRange = EnemyRoles::Mask(EEnemyRole::Pressure) | EnemyRoles::Mask(EEnemyRole::Flank);

	// Three melee enemies (pressure / flank) and a sniper (cover only): quotas 2 : 1 : 1 are reachable.
	const TArray<FCandidate> Group = {Only(CloseRange), Only(CloseRange), Only(CloseRange), Only(EnemyRoles::Mask(EEnemyRole::Cover))};
	const TArray<EEnemyRole> Roles = EnemyRoles::Assign(Group, DefaultWeights());
	TestEqual(TEXT("Sniper covers"), Roles[3], EEnemyRole::Cover);
	TestEqual(TEXT("Melee: pressure x2"), CountOf(Roles, EEnemyRole::Pressure), 2);
	TestEqual(TEXT("Melee: flank x1"), CountOf(Roles, EEnemyRole::Flank), 1);

	// Everyone may only press: the quota is exceeded because nobody can take the other roles.
	const TArray<EEnemyRole> Forced = EnemyRoles::Assign({Only(PressureOnly), Only(PressureOnly), Only(PressureOnly)}, DefaultWeights());
	TestEqual(TEXT("Pressure-only enemies all press"), CountOf(Forced, EEnemyRole::Pressure), 3);

	// Empty mask means "any role".
	TestEqual(TEXT("Empty mask = any"), EnemyRoles::Assign({Only(0)}, DefaultWeights())[0], EEnemyRole::Pressure);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesCoordinationRolesNoOvershootTest, "TypicalShooter.Enemies.Coordination.Roles.NoOvershootWhenAnotherCanCover",
	EnemiesCoordinationRolesTest::Flags)
bool FEnemiesCoordinationRolesNoOvershootTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesCoordinationRolesTest;
	// A flexible enemy holds the only pressure slot (3 enemies: 1 : 1 : 1); a newcomer may only press. The flexible one moves.
	const TArray<FCandidate> Group = {Holding(EEnemyRole::Pressure), Holding(EEnemyRole::Cover), Only(EnemyRoles::Mask(EEnemyRole::Pressure))};
	const TArray<EEnemyRole> Roles = EnemyRoles::Assign(Group, DefaultWeights());
	TestEqual(TEXT("Newcomer presses"), Roles[2], EEnemyRole::Pressure);
	TestEqual(TEXT("Flexible enemy flanks instead"), Roles[0], EEnemyRole::Flank);
	TestEqual(TEXT("Cover keeps its role"), Roles[1], EEnemyRole::Cover);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesCoordinationRolesStableTest, "TypicalShooter.Enemies.Coordination.Roles.StableOnRecompute",
	EnemiesCoordinationRolesTest::Flags)
bool FEnemiesCoordinationRolesStableTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesCoordinationRolesTest;
	// Current roles already match the quotas, in an order different from what a fresh assignment would give.
	const TArray<FCandidate> Group = {Holding(EEnemyRole::Cover), Holding(EEnemyRole::Flank), Holding(EEnemyRole::Pressure),
		Holding(EEnemyRole::Pressure)};
	const TArray<EEnemyRole> Roles = EnemyRoles::Assign(Group, DefaultWeights());
	TestEqual(TEXT("Nobody changes"), Changed(Group, Roles), 0);

	// A newcomer joins 3 enemies (1 : 1 : 1) -> quotas 2 : 1 : 1: it takes the free pressure slot.
	const TArray<FCandidate> Joined = {Holding(EEnemyRole::Pressure), Holding(EEnemyRole::Flank), Holding(EEnemyRole::Cover), Any()};
	const TArray<EEnemyRole> JoinedRoles = EnemyRoles::Assign(Joined, DefaultWeights());
	TestEqual(TEXT("Old members keep roles"), Changed(Joined, JoinedRoles), 0);
	TestEqual(TEXT("Newcomer presses"), JoinedRoles[3], EEnemyRole::Pressure);

	// A held role that is no longer allowed is dropped.
	const TArray<FCandidate> Disallowed = {Holding(EEnemyRole::Cover, EnemyRoles::Mask(EEnemyRole::Pressure))};
	TestEqual(TEXT("Disallowed role is replaced"), EnemyRoles::Assign(Disallowed, DefaultWeights())[0], EEnemyRole::Pressure);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesCoordinationRolesDeathTest, "TypicalShooter.Enemies.Coordination.Roles.MinimalChangeOnDeath",
	EnemiesCoordinationRolesTest::Flags)
bool FEnemiesCoordinationRolesDeathTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesCoordinationRolesTest;
	// 2 : 1 : 1, a pressure enemy dies -> 3 enemies 1 : 1 : 1: nobody has to change.
	const TArray<FCandidate> PressureDied = {Holding(EEnemyRole::Pressure), Holding(EEnemyRole::Flank), Holding(EEnemyRole::Cover)};
	TestEqual(TEXT("Pressure died: no changes"), Changed(PressureDied, EnemyRoles::Assign(PressureDied, DefaultWeights())), 0);

	// 2 : 1 : 1, the cover enemy dies -> 1 : 1 : 1: exactly one pressure enemy takes cover.
	const TArray<FCandidate> CoverDied = {Holding(EEnemyRole::Pressure), Holding(EEnemyRole::Pressure), Holding(EEnemyRole::Flank)};
	const TArray<EEnemyRole> Roles = EnemyRoles::Assign(CoverDied, DefaultWeights());
	TestEqual(TEXT("Cover died: one change"), Changed(CoverDied, Roles), 1);
	TestEqual(TEXT("Cover died: someone covers"), CountOf(Roles, EEnemyRole::Cover), 1);
	TestEqual(TEXT("Cover died: flank kept"), Roles[2], EEnemyRole::Flank);
	return true;
}

#endif
