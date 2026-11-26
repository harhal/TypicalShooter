// Tests for UEnemyDirectorSubsystem: registry of live enemies and target choice (nearest player pawn).

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Enemies/EnemyDirectorSubsystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Tests/EnemiesTestFixture.h"

namespace EnemiesDirectorTest
{
	/** Player controller possessing a character at Location. */
	APawn* SpawnPlayer(const ShootingTest::FWorldFixture& F, const FVector& Location)
	{
		APlayerController* Controller = F.World->SpawnActor<APlayerController>();
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ACharacter* Pawn = F.World->SpawnActor<ACharacter>(Location, FRotator::ZeroRotator, Params);
		Controller->Possess(Pawn);
		return Pawn;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesDirectorRegistryTest, "TypicalShooter.Enemies.Director.RegistersLiveEnemies", ShootingTest::Flags)
bool FEnemiesDirectorRegistryTest::RunTest(const FString& Parameters)
{
	const ShootingTest::FWorldFixture F(true);
	EnemiesTest::SpawnFloor(F);
	const UEnemyDirectorSubsystem* Director = F.World->GetSubsystem<UEnemyDirectorSubsystem>();
	TestNotNull(TEXT("The director exists in game worlds"), Director);
	if (!Director)
	{
		return false;
	}

	AEnemyCharacter* First = EnemiesTest::SpawnEnemy(F, FVector(0.f, 0.f, 0.f));
	AEnemyCharacter* Second = EnemiesTest::SpawnEnemy(F, FVector(0.f, 500.f, 0.f));
	TestEqual(TEXT("Spawned enemies are registered"), Director->GetEnemies().Num(), 2);

	EnemiesTest::Hit(First, 1000.f);
	TestEqual(TEXT("A dead enemy is unregistered at once"), Director->GetEnemies().Num(), 1);
	TestTrue(TEXT("The live one stays"), Director->GetEnemies().Contains(Second));

	Second->Destroy();
	TestEqual(TEXT("A destroyed enemy is unregistered"), Director->GetEnemies().Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesDirectorTargetTest, "TypicalShooter.Enemies.Director.NearestPlayerTarget", ShootingTest::Flags)
bool FEnemiesDirectorTargetTest::RunTest(const FString& Parameters)
{
	const ShootingTest::FWorldFixture F(true);
	EnemiesTest::SpawnFloor(F);
	const UEnemyDirectorSubsystem* Director = F.World->GetSubsystem<UEnemyDirectorSubsystem>();
	const AEnemyCharacter* Enemy = EnemiesTest::SpawnEnemy(F);
	TestNull(TEXT("No players: no target"), Director->FindTargetFor(Enemy));

	EnemiesDirectorTest::SpawnPlayer(F, FVector(3000.f, 0.f, 100.f));
	const APawn* Near = EnemiesDirectorTest::SpawnPlayer(F, FVector(-1000.f, 500.f, 100.f));
	TestTrue(TEXT("The nearest player pawn is the target"), Director->FindTargetFor(Enemy) == Near);
	return true;
}

#endif
