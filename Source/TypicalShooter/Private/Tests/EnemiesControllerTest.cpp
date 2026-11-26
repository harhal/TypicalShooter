// Tests for AEnemyAIController: the logic behind the StateTree tasks (track target, shoot when visible, keep distance).

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Enemies/EnemyAIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Tests/EnemiesTestFixture.h"

namespace EnemiesControllerTest
{
	struct FScene
	{
		AEnemyCharacter* Enemy = nullptr;
		AEnemyAIController* Controller = nullptr;
		ACharacter* Player = nullptr;

		/** Enemy at the origin (possessed by its AI controller), player character at PlayerLocation. */
		FScene(const ShootingTest::FWorldFixture& F, const FVector& PlayerLocation)
		{
			EnemiesTest::AddNavigation(F);
			EnemiesTest::SpawnFloor(F);
			Enemy = EnemiesTest::SpawnEnemy(F);
			Controller = Cast<AEnemyAIController>(Enemy->GetController());

			APlayerController* PlayerController = F.World->SpawnActor<APlayerController>();
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			Player = F.World->SpawnActor<ACharacter>(PlayerLocation + FVector(0.f, 0.f, 100.f), FRotator::ZeroRotator, Params);
			PlayerController->Possess(Player);
			F.Tick(0.2f); // land on the floor
		}
	};

	FEnemyStandSettings Settings()
	{
		FEnemyStandSettings S;
		S.MinDistance = 800.f;
		S.MaxDistance = 1600.f;
		return S;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesControllerPossessTest, "TypicalShooter.Enemies.Controller.PossessesSpawnedEnemy", ShootingTest::Flags)
bool FEnemiesControllerPossessTest::RunTest(const FString& Parameters)
{
	const ShootingTest::FWorldFixture F(true);
	const EnemiesControllerTest::FScene S(F, FVector(1200.f, 0.f, 0.f));
	TestNotNull(TEXT("A spawned enemy gets its AI controller"), S.Controller);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesControllerTargetTest, "TypicalShooter.Enemies.Controller.TracksAndShootsTarget", ShootingTest::Flags)
bool FEnemiesControllerTargetTest::RunTest(const FString& Parameters)
{
	const ShootingTest::FWorldFixture F(true);
	const EnemiesControllerTest::FScene S(F, FVector(1200.f, 300.f, 0.f));
	if (!S.Controller)
	{
		AddError(TEXT("No AI controller"));
		return false;
	}
	S.Controller->UpdateTarget();
	TestTrue(TEXT("Target = the player's pawn"), S.Controller->GetTarget() == S.Player);
	TestTrue(TEXT("Looks at the target"), S.Controller->GetFocusActor() == S.Player);

	S.Controller->UpdateFire();
	TestTrue(TEXT("Fires at the visible target"), S.Enemy->IsFiring());

	S.Controller->StopCombat();
	TestFalse(TEXT("Stops firing when combat stops"), S.Enemy->IsFiring());
	TestNull(TEXT("Stops looking at the target"), S.Controller->GetFocusActor());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesControllerDistanceTest, "TypicalShooter.Enemies.Controller.KeepsDistance", ShootingTest::Flags)
bool FEnemiesControllerDistanceTest::RunTest(const FString& Parameters)
{
	const ShootingTest::FWorldFixture F(true);
	const EnemiesControllerTest::FScene S(F, FVector(300.f, 0.f, 0.f));
	if (!S.Controller)
	{
		AddError(TEXT("No AI controller"));
		return false;
	}
	S.Controller->UpdateTarget();
	TestTrue(TEXT("Too close: starts moving"), S.Controller->UpdatePosition(EnemiesControllerTest::Settings()));
	// A lone enemy is the group's pressure enemy and holds the token: its band is the role's (500-900), not Settings().
	TestEqual(TEXT("Stand point in the middle of the pressure band"), FVector::Dist2D(S.Controller->GetStandPoint(), S.Player->GetActorLocation()), 700.0, 1.0);

	for (int32 Step = 0; Step < 300; ++Step)
	{
		S.Controller->UpdatePosition(EnemiesControllerTest::Settings());
		F.Tick(1.f / 60.f);
	}
	const double Distance = FVector::Dist2D(S.Enemy->GetActorLocation(), S.Player->GetActorLocation());
	TestTrue(FString::Printf(TEXT("Moved into the band (distance %.0f)"), Distance), Distance >= 450.0 && Distance <= 950.0);
	TestFalse(TEXT("Inside the band with a visible target: stays"), S.Controller->UpdatePosition(EnemiesControllerTest::Settings()));
	return true;
}

#endif
