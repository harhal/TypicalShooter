// Regression test: an enemy walking to its stand point steers around an ally standing on the way (it used to push into
// the ally's capsule forever, so the group never spread around the player).

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Enemies/EnemyAIController.h"
#include "Tests/EnemiesTestFixture.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesAvoidanceTest, "TypicalShooter.Enemies.Movement.AvoidsStandingAlly", ShootingTest::Flags)
bool FEnemiesAvoidanceTest::RunTest(const FString& Parameters)
{
	const ShootingTest::FWorldFixture F(true);
	EnemiesTest::AddNavigation(F);
	EnemiesTest::SpawnFloor(F);
	AEnemyCharacter* Walker = EnemiesTest::SpawnEnemy(F, FVector(-1200.f, 0.f, 0.f));
	EnemiesTest::SpawnEnemy(F, FVector(-600.f, 0.f, 0.f)); // stands right on the walker's line
	AEnemyAIController* Controller = Cast<AEnemyAIController>(Walker->GetController());
	if (!TestNotNull(TEXT("AI controller"), Controller))
	{
		return false;
	}
	F.Tick(0.2f);

	const FVector Goal(0.f, 0.f, Walker->GetActorLocation().Z);
	Controller->MoveToLocation(Goal, 100.f, true, false, false, true);
	F.Tick(5.f);
	const double Left = FVector::Dist2D(Walker->GetActorLocation(), Goal);
	TestTrue(FString::Printf(TEXT("Got around the ally to the goal (%.0f cm left)"), Left), Left <= 150.0);
	return true;
}

#endif
