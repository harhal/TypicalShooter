// Tests for the enemy / controller side of the coordination: attack only with a token, stand in the role's band and the
// sector, allowed roles and per-class band overrides (R4, R8, R10, R18).

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Enemies/Coordination/EnemyCoordinationSettings.h"
#include "Enemies/EnemyAIController.h"
#include "Enemies/EnemyDirectorSubsystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Tests/EnemiesTestFixture.h"

namespace EnemiesControllerCoordinationTest
{
	ACharacter* SpawnPlayer(const ShootingTest::FWorldFixture& F, const FVector& Location)
	{
		APlayerController* Controller = F.World->SpawnActor<APlayerController>();
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ACharacter* Pawn = F.World->SpawnActor<ACharacter>(Location + FVector(0.f, 0.f, 100.f), FRotator::ZeroRotator, Params);
		Controller->Possess(Pawn);
		return Pawn;
	}

	/** Enemy whose class defaults are tweaked by Setup before BeginPlay. */
	AEnemyCharacter* SpawnConfigured(const ShootingTest::FWorldFixture& F, const FVector& Location,
		const TFunctionRef<void(AEnemyCharacter&)> Setup)
	{
		const FTransform Transform(FRotator::ZeroRotator, Location + FVector(0.f, 0.f, 100.f));
		AEnemyCharacter* Enemy = F.World->SpawnActorDeferred<AEnemyCharacter>(AEnemyCharacter::StaticClass(), Transform, nullptr,
			nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		Setup(*Enemy);
		Enemy->FinishSpawning(Transform);
		return Enemy;
	}

	AEnemyAIController* ControllerOf(const AEnemyCharacter* Enemy)
	{
		return Enemy ? Cast<AEnemyAIController>(Enemy->GetController()) : nullptr;
	}

	void Prepare(const ShootingTest::FWorldFixture& F)
	{
		EnemiesTest::AddNavigation(F);
		EnemiesTest::SpawnFloor(F);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesControllerTokenTest, "TypicalShooter.Enemies.Controller.FiresOnlyWithToken", ShootingTest::Flags)
bool FEnemiesControllerTokenTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesControllerCoordinationTest;
	const ShootingTest::FWorldFixture F(true);
	Prepare(F);
	SpawnPlayer(F, FVector(1200.f, 300.f, 0.f));
	const TArray<AEnemyCharacter*> Enemies = {EnemiesTest::SpawnEnemy(F, FVector(0.f, 0.f, 0.f)), EnemiesTest::SpawnEnemy(F, FVector(0.f, 600.f, 0.f))};
	F.Tick(0.2f);
	const UEnemyDirectorSubsystem* Director = F.World->GetSubsystem<UEnemyDirectorSubsystem>();
	TestEqual(TEXT("Two enemies share one token"), GetDefault<UEnemyCoordinationSettings>()->GetTokenCount(2), 1);

	for (const AEnemyCharacter* Enemy : Enemies)
	{
		if (!TestNotNull(TEXT("AI controller"), ControllerOf(Enemy)))
		{
			return false;
		}
		ControllerOf(Enemy)->UpdateTarget();
		ControllerOf(Enemy)->UpdateFire();
		TestTrue(TEXT("Both see the player"), Enemy->CanAttack(ControllerOf(Enemy)->GetTarget()));
	}
	const int32 HolderIndex = Director->HasAttackToken(Enemies[0]) ? 0 : 1;
	AEnemyCharacter* Holder = Enemies[HolderIndex];
	AEnemyCharacter* Other = Enemies[1 - HolderIndex];
	TestTrue(TEXT("The token holder fires"), Holder->IsFiring());
	TestFalse(TEXT("The other one sees the player but waits"), Other->IsFiring());
	TestTrue(TEXT("Controller reports the token"), ControllerOf(Holder)->HasAttackToken());

	ControllerOf(Holder)->StopCombat();
	TestFalse(TEXT("Leaving combat gives the token back"), Director->HasAttackToken(Holder));
	F.Tick(GetDefault<UEnemyCoordinationSettings>()->UpdateInterval + 0.1f);
	ControllerOf(Other)->UpdateFire();
	TestTrue(TEXT("The other one gets the token and fires"), Other->IsFiring());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesControllerRoleBandTest, "TypicalShooter.Enemies.Controller.RoleBand", ShootingTest::Flags)
bool FEnemiesControllerRoleBandTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesControllerCoordinationTest;
	const ShootingTest::FWorldFixture F(true);
	Prepare(F);
	const ACharacter* Player = SpawnPlayer(F, FVector(400.f, 0.f, 0.f));
	AEnemyCharacter* Enemy = SpawnConfigured(F, FVector::ZeroVector, [](AEnemyCharacter& E) { E.AllowedRoles = {EEnemyRole::Cover}; });
	F.Tick(0.2f);
	const UEnemyDirectorSubsystem* Director = F.World->GetSubsystem<UEnemyDirectorSubsystem>();
	const FEnemyAssignment* Assignment = Director->GetAssignment(Enemy);
	if (!TestNotNull(TEXT("Assigned"), Assignment) || !TestNotNull(TEXT("AI controller"), ControllerOf(Enemy)))
	{
		return false;
	}
	TestEqual(TEXT("Only cover is allowed"), Assignment->Role, EEnemyRole::Cover);

	AEnemyAIController* Controller = ControllerOf(Enemy);
	Controller->UpdateTarget();
	TestTrue(TEXT("Too close for cover: moves"), Controller->UpdatePosition(FEnemyStandSettings()));
	const FEnemyDistanceBand Band = GetDefault<UEnemyCoordinationSettings>()->RoleBands.Cover;
	TestEqual(TEXT("Stand point in the middle of the cover band"), float(FVector::Dist2D(Controller->GetStandPoint(), Player->GetActorLocation())),
		0.5f * (Band.Min + Band.Max), 1.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesControllerBandOverrideTest, "TypicalShooter.Enemies.Controller.RoleBandOverride", ShootingTest::Flags)
bool FEnemiesControllerBandOverrideTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesControllerCoordinationTest;
	const ShootingTest::FWorldFixture F(true);
	Prepare(F);
	const ACharacter* Player = SpawnPlayer(F, FVector(1200.f, 0.f, 0.f));
	// A melee-like enemy: pressure / flank only, right next to the target.
	AEnemyCharacter* Enemy = SpawnConfigured(F, FVector::ZeroVector, [](AEnemyCharacter& E)
	{
		E.AllowedRoles = {EEnemyRole::Pressure, EEnemyRole::Flank};
		E.RoleBandOverrides.Add(EEnemyRole::Pressure, FEnemyDistanceBand(0.f, 150.f));
	});
	F.Tick(0.2f);
	TestEqual(TEXT("Override for pressure"), Enemy->GetRoleBand(EEnemyRole::Pressure).Max, 150.f);
	TestEqual(TEXT("No override for flank: project band"), Enemy->GetRoleBand(EEnemyRole::Flank).Max,
		GetDefault<UEnemyCoordinationSettings>()->RoleBands.Flank.Max);

	AEnemyAIController* Controller = ControllerOf(Enemy);
	if (!TestNotNull(TEXT("AI controller"), Controller))
	{
		return false;
	}
	Controller->UpdateTarget();
	TestTrue(TEXT("Far from the target: moves"), Controller->UpdatePosition(FEnemyStandSettings()));
	TestEqual(TEXT("Stand point right next to the target"), float(FVector::Dist2D(Controller->GetStandPoint(), Player->GetActorLocation())), 75.f, 1.f);
	return true;
}

#endif
