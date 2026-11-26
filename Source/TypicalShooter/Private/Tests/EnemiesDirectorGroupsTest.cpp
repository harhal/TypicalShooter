// Tests for the group coordination in UEnemyDirectorSubsystem: groups per target, roles / sectors / tokens per group,
// immediate recompute on death, following target changes.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Algo/AllOf.h"
#include "Algo/Count.h"
#include "Enemies/Coordination/EnemyCoordinationSettings.h"
#include "Enemies/EnemyDirectorSubsystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Tests/EnemiesTestFixture.h"

namespace EnemiesDirectorGroupsTest
{
	APawn* SpawnPlayer(const ShootingTest::FWorldFixture& F, const FVector& Location)
	{
		APlayerController* Controller = F.World->SpawnActor<APlayerController>();
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ACharacter* Pawn = F.World->SpawnActor<ACharacter>(Location + FVector(0.f, 0.f, 100.f), FRotator::ZeroRotator, Params);
		Controller->Possess(Pawn);
		return Pawn;
	}

	/** Enemies on a circle of Radius around Center at the given yaws, facing the center. */
	TArray<AEnemyCharacter*> SpawnAround(const ShootingTest::FWorldFixture& F, const FVector& Center, const float Radius,
		const TArray<float>& Yaws)
	{
		TArray<AEnemyCharacter*> Enemies;
		for (const float Yaw : Yaws)
		{
			const FVector Location = Center + FVector(Radius, 0.f, 0.f).RotateAngleAxis(Yaw, FVector::UpVector);
			Enemies.Add(EnemiesTest::SpawnEnemy(F, Location, Yaw + 180.f));
		}
		return Enemies;
	}

	int32 CountRole(const UEnemyDirectorSubsystem& Director, const TArray<AEnemyCharacter*>& Enemies, const EEnemyRole Role)
	{
		return Algo::CountIf(Enemies, [&Director, Role](const AEnemyCharacter* Enemy)
		{
			const FEnemyAssignment* Assignment = Director.GetAssignment(Enemy);
			return Assignment && Assignment->Role == Role;
		});
	}

	bool AllAssigned(const UEnemyDirectorSubsystem& Director, const TArray<AEnemyCharacter*>& Enemies)
	{
		return Algo::AllOf(Enemies, [&Director](const AEnemyCharacter* Enemy) { return Director.GetAssignment(Enemy) != nullptr; });
	}

	int32 CountTokens(const UEnemyDirectorSubsystem& Director, const TArray<AEnemyCharacter*>& Enemies)
	{
		return Algo::CountIf(Enemies, [&Director](const AEnemyCharacter* Enemy) { return Director.HasAttackToken(Enemy); });
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesDirectorTwoTargetsTest, "TypicalShooter.Enemies.Director.GroupsTwoTargets", ShootingTest::Flags)
bool FEnemiesDirectorTwoTargetsTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesDirectorGroupsTest;
	const ShootingTest::FWorldFixture F(true);
	EnemiesTest::SpawnFloor(F, FVector(20000.f, 5000.f, 10.f));
	const APawn* A = SpawnPlayer(F, FVector::ZeroVector);
	const APawn* B = SpawnPlayer(F, FVector(15000.f, 0.f, 0.f));
	const TArray<AEnemyCharacter*> NearA = SpawnAround(F, FVector::ZeroVector, 1000.f, {0.f, 120.f, -120.f});
	const TArray<AEnemyCharacter*> NearB = SpawnAround(F, FVector(15000.f, 0.f, 0.f), 1000.f, {0.f});

	const UEnemyDirectorSubsystem* Director = F.World->GetSubsystem<UEnemyDirectorSubsystem>();
	TestEqual(TEXT("A group per target"), Director->GetGroupTargets().Num(), 2);
	TestEqual(TEXT("Group of A"), Director->GetGroupMembers(A).Num(), 3);
	TestEqual(TEXT("Group of B"), Director->GetGroupMembers(B).Num(), 1);
	TestTrue(TEXT("Assignment knows its target"), Director->GetAssignment(NearA[0]) && Director->GetAssignment(NearA[0])->Target == A);

	TestEqual(TEXT("A: one pressure"), CountRole(*Director, NearA, EEnemyRole::Pressure), 1);
	TestEqual(TEXT("A: one flank"), CountRole(*Director, NearA, EEnemyRole::Flank), 1);
	TestEqual(TEXT("A: one cover"), CountRole(*Director, NearA, EEnemyRole::Cover), 1);
	TestEqual(TEXT("B: the lone enemy presses"), CountRole(*Director, NearB, EEnemyRole::Pressure), 1);
	TestEqual(TEXT("B: its own token (group of B coordinates apart)"), CountTokens(*Director, NearB), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesDirectorSpreadTest, "TypicalShooter.Enemies.Director.GroupsSpreadAndShareTokens", ShootingTest::Flags)
bool FEnemiesDirectorSpreadTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesDirectorGroupsTest;
	const ShootingTest::FWorldFixture F(true);
	EnemiesTest::SpawnFloor(F);
	SpawnPlayer(F, FVector::ZeroVector);
	const TArray<AEnemyCharacter*> Enemies = SpawnAround(F, FVector::ZeroVector, 1200.f, {0.f, 10.f, 20.f, 30.f});
	UEnemyDirectorSubsystem* Director = F.World->GetSubsystem<UEnemyDirectorSubsystem>();
	if (!TestTrue(TEXT("Every enemy is assigned"), AllAssigned(*Director, Enemies)))
	{
		return false;
	}

	float MinGap = 360.f;
	for (int32 I = 0; I < Enemies.Num(); ++I)
	{
		for (int32 J = I + 1; J < Enemies.Num(); ++J)
		{
			MinGap = FMath::Min(MinGap, FMath::Abs(FMath::FindDeltaAngleDegrees(Director->GetAssignment(Enemies[I])->SectorYaw,
				Director->GetAssignment(Enemies[J])->SectorYaw)));
		}
	}
	TestTrue(FString::Printf(TEXT("The clump gets four quarters (min gap %.1f)"), MinGap), MinGap > 89.f);
	TestEqual(TEXT("Sector half width"), Director->GetAssignment(Enemies[0])->SectorHalfWidth, 45.f);

	const int32 Tokens = GetDefault<UEnemyCoordinationSettings>()->GetTokenCount(4);
	TestEqual(TEXT("Tokens shared by the group"), CountTokens(*Director, Enemies), Tokens);

	AEnemyCharacter* const* HolderPtr = Enemies.FindByPredicate([Director](const AEnemyCharacter* E) { return Director->HasAttackToken(E); });
	if (!TestNotNull(TEXT("Someone holds a token"), HolderPtr))
	{
		return false;
	}
	AEnemyCharacter* Holder = *HolderPtr;
	Director->ReleaseAttackToken(Holder);
	TestFalse(TEXT("Released at once"), Director->HasAttackToken(Holder));
	F.Tick(GetDefault<UEnemyCoordinationSettings>()->UpdateInterval + 0.1f);
	TestEqual(TEXT("The next update hands the token on"), CountTokens(*Director, Enemies), Tokens);
	TestFalse(TEXT("Not back to the one that just released it (cooldown)"), Director->HasAttackToken(Holder));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesDirectorDeathTest, "TypicalShooter.Enemies.Director.RecomputeOnDeath", ShootingTest::Flags)
bool FEnemiesDirectorDeathTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesDirectorGroupsTest;
	const ShootingTest::FWorldFixture F(true);
	EnemiesTest::SpawnFloor(F);
	const APawn* Player = SpawnPlayer(F, FVector::ZeroVector);
	TArray<AEnemyCharacter*> Enemies = SpawnAround(F, FVector::ZeroVector, 1200.f, {0.f, 90.f, 180.f, -90.f});
	const UEnemyDirectorSubsystem* Director = F.World->GetSubsystem<UEnemyDirectorSubsystem>();
	TestEqual(TEXT("2 : 1 : 1 before"), CountRole(*Director, Enemies, EEnemyRole::Pressure), 2);

	AEnemyCharacter* const* VictimPtr = Enemies.FindByPredicate([Director](const AEnemyCharacter* E) { return Director->HasAttackToken(E); });
	if (!TestNotNull(TEXT("Someone holds a token"), VictimPtr))
	{
		return false;
	}
	AEnemyCharacter* Victim = *VictimPtr;
	EnemiesTest::Hit(Victim, 1000.f);
	Enemies.Remove(Victim);

	TestNull(TEXT("The dead enemy has no assignment"), Director->GetAssignment(Victim));
	TestEqual(TEXT("Group shrank at once"), Director->GetGroupMembers(Player).Num(), 3);
	TestEqual(TEXT("1 : 1 : 1 after - pressure"), CountRole(*Director, Enemies, EEnemyRole::Pressure), 1);
	TestEqual(TEXT("1 : 1 : 1 after - cover"), CountRole(*Director, Enemies, EEnemyRole::Cover), 1);
	TestEqual(TEXT("Tokens for 3"), CountTokens(*Director, Enemies), GetDefault<UEnemyCoordinationSettings>()->GetTokenCount(3));
	if (TestTrue(TEXT("Survivors stay assigned"), AllAssigned(*Director, Enemies)))
	{
		TestEqual(TEXT("Sectors for 3"), Director->GetAssignment(Enemies[0])->SectorHalfWidth, 60.f);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesDirectorTargetChangeTest, "TypicalShooter.Enemies.Director.FollowsTargetChange", ShootingTest::Flags)
bool FEnemiesDirectorTargetChangeTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesDirectorGroupsTest;
	const ShootingTest::FWorldFixture F(true);
	EnemiesTest::SpawnFloor(F);
	const APawn* A = SpawnPlayer(F, FVector::ZeroVector);
	AEnemyCharacter* Enemy = EnemiesTest::SpawnEnemy(F, FVector(1000.f, 0.f, 0.f));
	const UEnemyDirectorSubsystem* Director = F.World->GetSubsystem<UEnemyDirectorSubsystem>();
	TestTrue(TEXT("Starts in A's group"), Director->GetAssignment(Enemy) && Director->GetAssignment(Enemy)->Target == A);

	const APawn* B = SpawnPlayer(F, FVector(1500.f, 0.f, 0.f));
	F.Tick(0.05f); // well below the update interval
	TestTrue(TEXT("Moves to the nearer player's group on the next tick"), Director->GetAssignment(Enemy) && Director->GetAssignment(Enemy)->Target == B);
	TestEqual(TEXT("A has no group left"), Director->GetGroupMembers(A).Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesDirectorOutOfViewTokenTest, "TypicalShooter.Enemies.Director.OutOfViewHoldsTokenShorter", ShootingTest::Flags)
bool FEnemiesDirectorOutOfViewTokenTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesDirectorGroupsTest;
	const ShootingTest::FWorldFixture F(true);
	EnemiesTest::SpawnFloor(F);
	SpawnPlayer(F, FVector::ZeroVector); // looks along +X
	TArray<AEnemyCharacter*> Pair;
	for (const FVector& Location : {FVector(-900.f, 0.f, 0.f), FVector(1000.f, 0.f, 0.f)}) // behind (nearer), in front
	{
		const FTransform Transform(FRotator::ZeroRotator, Location + FVector(0.f, 0.f, 100.f));
		AEnemyCharacter* Enemy = F.World->SpawnActorDeferred<AEnemyCharacter>(AEnemyCharacter::StaticClass(), Transform, nullptr,
			nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		Enemy->AllowedRoles = {EEnemyRole::Pressure};
		Enemy->FinishSpawning(Transform);
		Pair.Add(Enemy);
	}
	const UEnemyDirectorSubsystem* Director = F.World->GetSubsystem<UEnemyDirectorSubsystem>();
	F.Tick(0.1f);
	TestTrue(TEXT("The nearer one (behind the player) gets the single token"), Director->HasAttackToken(Pair[0]));

	// In view it would hold 3 s; out of view half of that. Updates run every 0.5 s.
	const FEnemyTokenRotationSettings& Rotation = GetDefault<UEnemyCoordinationSettings>()->TokenRotation;
	F.Tick(Rotation.TokenHoldTime * Rotation.OutOfViewHoldScale + GetDefault<UEnemyCoordinationSettings>()->UpdateInterval + 0.1f);
	TestTrue(TEXT("Out of the player's view it passes the token on early"), Director->HasAttackToken(Pair[1]));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesDirectorLoneFlankerTest, "TypicalShooter.Enemies.Director.LoneFlankerLeavesView", ShootingTest::Flags)
bool FEnemiesDirectorLoneFlankerTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesDirectorGroupsTest;
	const ShootingTest::FWorldFixture F(true);
	EnemiesTest::SpawnFloor(F);
	SpawnPlayer(F, FVector::ZeroVector); // looks along +X
	const FTransform Transform(FRotator::ZeroRotator, FVector(1000.f, 0.f, 100.f)); // right in front of the player
	AEnemyCharacter* Flanker = F.World->SpawnActorDeferred<AEnemyCharacter>(AEnemyCharacter::StaticClass(), Transform, nullptr,
		nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	Flanker->AllowedRoles = {EEnemyRole::Flank};
	Flanker->FinishSpawning(Transform);

	const FEnemyAssignment* Assignment = F.World->GetSubsystem<UEnemyDirectorSubsystem>()->GetAssignment(Flanker);
	if (!TestNotNull(TEXT("Assigned"), Assignment))
	{
		return false;
	}
	const float ViewHalf = GetDefault<UEnemyCoordinationSettings>()->Sectors.FlankViewHalfAngle;
	TestEqual(TEXT("Its sector is behind the player"), FMath::Abs(FMath::FindDeltaAngleDegrees(Assignment->SectorYaw, 180.f)), 0.f, 1.f);
	TestTrue(FString::Printf(TEXT("The sector stays out of the view cone (half width %.0f <= %.0f)"), Assignment->SectorHalfWidth, 180.f - ViewHalf),
		Assignment->SectorHalfWidth <= 180.f - ViewHalf + 0.01f);
	return true;
}

#endif
