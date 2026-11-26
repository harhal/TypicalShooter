// Tests for FEnemyTokenPool: who may attack at a moment - visible only, role / waiting / distance priority, hold time
// rotation, lost sight grace, cooldown.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Enemies/Coordination/EnemyCoordinationSettings.h"
#include "Enemies/Coordination/EnemyTokenPool.h"

namespace EnemiesCoordinationTokensTest
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	FEnemyTokenRotationSettings Rotation()
	{
		FEnemyTokenRotationSettings S;
		S.TokenHoldTime = 3.f;
		S.LostSightGrace = 0.5f;
		S.TokenCooldown = 1.5f;
		return S;
	}

	FEnemyTokenCandidate Candidate(const int32 Id, const float Distance, const EEnemyRole Role = EEnemyRole::Pressure,
		const bool bSees = true, const bool bInView = true)
	{
		FEnemyTokenCandidate C;
		C.Id = Id;
		C.Role = Role;
		C.bSeesTarget = bSees;
		C.DistanceToTarget = Distance;
		C.bInTargetView = bInView;
		return C;
	}

	FEnemyTokenRotationSettings RotationWithView()
	{
		FEnemyTokenRotationSettings S = Rotation();
		S.OutOfViewHoldScale = 0.5f;
		S.OutOfViewCooldownScale = 3.f;
		return S;
	}

	FString Holders(const FEnemyTokenPool& Pool)
	{
		TArray<int32> Ids = Pool.GetHolders();
		Ids.Sort();
		return FString::JoinBy(Ids, TEXT(","), [](const int32 Id) { return FString::FromInt(Id); });
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesCoordinationTokensVisibleTest, "TypicalShooter.Enemies.Coordination.Tokens.OnlyVisible",
	EnemiesCoordinationTokensTest::Flags)
bool FEnemiesCoordinationTokensVisibleTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesCoordinationTokensTest;
	FEnemyTokenPool Pool;
	Pool.Update(0.f, {Candidate(1, 100.f, EEnemyRole::Pressure, false), Candidate(2, 500.f), Candidate(3, 900.f)}, 2, Rotation());
	TestEqual(TEXT("Only enemies that see the target"), Holders(Pool), FString(TEXT("2,3")));
	TestFalse(TEXT("HasToken: blind enemy"), Pool.HasToken(1));
	TestTrue(TEXT("HasToken: visible enemy"), Pool.HasToken(2));

	Pool.Update(0.1f, {Candidate(2, 500.f), Candidate(3, 900.f)}, 1, Rotation());
	TestEqual(TEXT("Fewer tokens: the lower priority holder gives one back"), Holders(Pool), FString(TEXT("2")));

	Pool.Update(0.2f, {Candidate(3, 900.f)}, 1, Rotation());
	TestEqual(TEXT("A holder leaving the group frees its token"), Holders(Pool), FString(TEXT("3")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesCoordinationTokensPriorityTest, "TypicalShooter.Enemies.Coordination.Tokens.Priority",
	EnemiesCoordinationTokensTest::Flags)
bool FEnemiesCoordinationTokensPriorityTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesCoordinationTokensTest;
	FEnemyTokenPool Pool;
	Pool.Update(0.f, {Candidate(1, 100.f, EEnemyRole::Cover), Candidate(2, 1500.f, EEnemyRole::Pressure),
		Candidate(3, 200.f, EEnemyRole::Flank), Candidate(4, 600.f, EEnemyRole::Pressure)}, 2, Rotation());
	TestEqual(TEXT("Role first: both pressure enemies"), Holders(Pool), FString(TEXT("2,4")));

	FEnemyTokenPool Near;
	Near.Update(0.f, {Candidate(1, 900.f), Candidate(2, 300.f)}, 1, Rotation());
	TestEqual(TEXT("Same role and waiting time: the nearer one"), Holders(Near), FString(TEXT("2")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesCoordinationTokensRotationTest, "TypicalShooter.Enemies.Coordination.Tokens.RotationAfterHoldTime",
	EnemiesCoordinationTokensTest::Flags)
bool FEnemiesCoordinationTokensRotationTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesCoordinationTokensTest;
	const TArray<FEnemyTokenCandidate> Group = {Candidate(1, 100.f), Candidate(2, 200.f), Candidate(3, 300.f)};
	FEnemyTokenPool Pool;
	Pool.Update(0.f, Group, 1, Rotation());
	TestEqual(TEXT("t=0: nearest"), Holders(Pool), FString(TEXT("1")));
	Pool.Update(2.9f, Group, 1, Rotation());
	TestEqual(TEXT("t=2.9: still held"), Holders(Pool), FString(TEXT("1")));
	Pool.Update(3.1f, Group, 1, Rotation());
	TestEqual(TEXT("t=3.1: hold time over, next nearest of the waiting"), Holders(Pool), FString(TEXT("2")));
	Pool.Update(6.2f, Group, 1, Rotation());
	TestEqual(TEXT("t=6.2: the one waiting longest (since 0) beats the nearer one (since 3.1)"), Holders(Pool), FString(TEXT("3")));

	FEnemyTokenPool Alone;
	Alone.Update(0.f, {Candidate(1, 100.f)}, 1, Rotation());
	Alone.Update(5.f, {Candidate(1, 100.f), Candidate(2, 100.f, EEnemyRole::Pressure, false)}, 1, Rotation());
	TestEqual(TEXT("Nobody else can attack: keeps the token past the hold time"), Holders(Alone), FString(TEXT("1")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesCoordinationTokensSightTest, "TypicalShooter.Enemies.Coordination.Tokens.LostSightGrace",
	EnemiesCoordinationTokensTest::Flags)
bool FEnemiesCoordinationTokensSightTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesCoordinationTokensTest;
	const FEnemyTokenCandidate Waiting = Candidate(2, 500.f);
	FEnemyTokenPool Pool;
	Pool.Update(0.f, {Candidate(1, 100.f), Waiting}, 1, Rotation());
	Pool.Update(1.f, {Candidate(1, 100.f, EEnemyRole::Pressure, false), Waiting}, 1, Rotation());
	Pool.Update(1.4f, {Candidate(1, 100.f, EEnemyRole::Pressure, false), Waiting}, 1, Rotation());
	TestEqual(TEXT("Lost sight 0.4 s ago: still holds"), Holders(Pool), FString(TEXT("1")));
	Pool.Update(1.45f, {Candidate(1, 100.f), Waiting}, 1, Rotation());
	Pool.Update(2.2f, {Candidate(1, 100.f, EEnemyRole::Pressure, false), Waiting}, 1, Rotation());
	Pool.Update(2.6f, {Candidate(1, 100.f, EEnemyRole::Pressure, false), Waiting}, 1, Rotation());
	TestEqual(TEXT("Seeing again reset the grace"), Holders(Pool), FString(TEXT("1")));
	Pool.Update(2.8f, {Candidate(1, 100.f, EEnemyRole::Pressure, false), Waiting}, 1, Rotation());
	TestEqual(TEXT("Lost sight 0.6 s ago: token passes on"), Holders(Pool), FString(TEXT("2")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesCoordinationTokensCooldownTest, "TypicalShooter.Enemies.Coordination.Tokens.CooldownAfterRelease",
	EnemiesCoordinationTokensTest::Flags)
bool FEnemiesCoordinationTokensCooldownTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesCoordinationTokensTest;
	const TArray<FEnemyTokenCandidate> Group = {Candidate(1, 100.f, EEnemyRole::Pressure), Candidate(2, 900.f, EEnemyRole::Cover)};
	FEnemyTokenPool Pool;
	Pool.Update(0.f, Group, 1, Rotation());
	TestEqual(TEXT("Pressure first"), Holders(Pool), FString(TEXT("1")));

	Pool.Release(1, 1.f); // e.g. left the combat state
	TestEqual(TEXT("Released at once"), Holders(Pool), FString());
	Pool.Update(1.f, Group, 1, Rotation());
	TestEqual(TEXT("On cooldown: the cover enemy gets it despite the lower role"), Holders(Pool), FString(TEXT("2")));

	FEnemyTokenPool Single;
	Single.Update(0.f, {Candidate(1, 100.f)}, 1, Rotation());
	Single.Release(1, 1.f);
	Single.Update(1.1f, {Candidate(1, 100.f)}, 1, Rotation());
	TestEqual(TEXT("No other claimant: cooldown does not block"), Holders(Single), FString(TEXT("1")));

	const FEnemyTokenRotationSettings& Project = GetDefault<UEnemyCoordinationSettings>()->TokenRotation;
	TestEqual(TEXT("Default hold time"), Project.TokenHoldTime, 3.f);
	TestEqual(TEXT("Default lost sight grace"), Project.LostSightGrace, 0.5f);
	TestEqual(TEXT("Default cooldown"), Project.TokenCooldown, 1.5f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesCoordinationTokensOutOfViewHoldTest, "TypicalShooter.Enemies.Coordination.Tokens.OutOfViewHoldsShorter",
	EnemiesCoordinationTokensTest::Flags)
bool FEnemiesCoordinationTokensOutOfViewHoldTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesCoordinationTokensTest;
	// Behind the player, nearer, so it gets the token first; the other one waits in front of him.
	const TArray<FEnemyTokenCandidate> Group = {Candidate(1, 100.f, EEnemyRole::Pressure, true, false), Candidate(2, 500.f)};
	FEnemyTokenPool Pool;
	Pool.Update(0.f, Group, 1, RotationWithView());
	TestEqual(TEXT("t=0: the nearer one (out of view)"), Holders(Pool), FString(TEXT("1")));
	Pool.Update(1.4f, Group, 1, RotationWithView());
	TestEqual(TEXT("t=1.4: still held"), Holders(Pool), FString(TEXT("1")));
	Pool.Update(1.6f, Group, 1, RotationWithView());
	TestEqual(TEXT("t=1.6: out of view it holds only half the hold time"), Holders(Pool), FString(TEXT("2")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesCoordinationTokensOutOfViewCooldownTest, "TypicalShooter.Enemies.Coordination.Tokens.OutOfViewLongerCooldown",
	EnemiesCoordinationTokensTest::Flags)
bool FEnemiesCoordinationTokensOutOfViewCooldownTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesCoordinationTokensTest;
	for (const bool bPresserInView : {true, false})
	{
		// A pressure enemy (first in line by role) and two cover enemies in view; one token.
		const TArray<FEnemyTokenCandidate> Group = {Candidate(1, 100.f, EEnemyRole::Pressure, true, bPresserInView),
			Candidate(2, 900.f, EEnemyRole::Cover), Candidate(3, 1000.f, EEnemyRole::Cover)};
		FEnemyTokenPool Pool;
		Pool.Update(0.f, Group, 1, RotationWithView());
		Pool.Release(1, 1.f);
		Pool.Update(1.f, Group, 1, RotationWithView());
		Pool.Release(2, 3.f); // the presser released 2 s ago: past the 1.5 s cooldown, not past 3 x 1.5 s
		Pool.Update(3.f, Group, 1, RotationWithView());
		TestEqual(bPresserInView ? TEXT("In view: the presser is back first") : TEXT("Out of view: still yields to a fresh enemy"),
			Holders(Pool), FString(bPresserInView ? TEXT("1") : TEXT("3")));
	}

	const FEnemyTokenRotationSettings& Project = GetDefault<UEnemyCoordinationSettings>()->TokenRotation;
	TestEqual(TEXT("Default out-of-view hold scale"), Project.OutOfViewHoldScale, 0.5f);
	TestEqual(TEXT("Default out-of-view cooldown scale"), Project.OutOfViewCooldownScale, 3.f);
	return true;
}

#endif
