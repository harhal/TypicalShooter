// Regression test: a stand point must leave room for the enemy's capsule next to level geometry. Points hugging a wall
// let enemies wedge themselves into it (seen on Lvl_FirstPerson: a cover enemy stuck in the back wall for good).

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Enemies/EnemyAIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Tests/EnemiesTestFixture.h"

namespace EnemiesStandClearanceTest
{
	/** Static wall whose face (towards +X) is at FaceX, spanning Y and Z. */
	void SpawnWall(const ShootingTest::FWorldFixture& F, const float FaceX)
	{
		AActor* Wall = F.World->SpawnActor<AActor>();
		UBoxComponent* Box = NewObject<UBoxComponent>(Wall);
		Box->SetBoxExtent(FVector(100.f, 3000.f, 500.f));
		Box->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
		Wall->SetRootComponent(Box);
		Box->RegisterComponent();
		Box->SetWorldLocation(FVector(FaceX - 100.f, 0.f, 500.f));
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesStandClearanceTest, "TypicalShooter.Enemies.Controller.StandPointClearOfWalls", ShootingTest::Flags)
bool FEnemiesStandClearanceTest::RunTest(const FString& Parameters)
{
	const ShootingTest::FWorldFixture F(true);
	EnemiesTest::AddNavigation(F);
	EnemiesTest::SpawnFloor(F);
	// The lone enemy presses (band 500-900, middle 700); straight back from the player it would stand at X = -400,
	// 10 cm from the wall face at X = -410.
	EnemiesStandClearanceTest::SpawnWall(F, -410.f);
	AEnemyCharacter* Enemy = EnemiesTest::SpawnEnemy(F, FVector::ZeroVector);
	APlayerController* PlayerController = F.World->SpawnActor<APlayerController>();
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	PlayerController->Possess(F.World->SpawnActor<ACharacter>(FVector(300.f, 0.f, 100.f), FRotator::ZeroRotator, Params));
	F.Tick(0.2f);

	AEnemyAIController* Controller = Cast<AEnemyAIController>(Enemy->GetController());
	if (!TestNotNull(TEXT("AI controller"), Controller))
	{
		return false;
	}
	Controller->UpdateTarget();
	TestTrue(TEXT("Too close: moves"), Controller->UpdatePosition(FEnemyStandSettings()));
	const float Radius = Enemy->GetCapsuleComponent()->GetScaledCapsuleRadius();
	const double FromWall = Controller->GetStandPoint().X - (-410.0);
	TestTrue(FString::Printf(TEXT("Stand point leaves room for the capsule (%.0f cm from the wall, radius %.0f)"), FromWall, Radius),
		FromWall >= Radius + 10.0);
	return true;
}

#endif
