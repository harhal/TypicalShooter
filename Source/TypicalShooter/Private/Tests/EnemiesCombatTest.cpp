// Tests for the enemy's fire decision: shoot only at a target in range with a clear line from the muzzle.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Shooting/TargetDummy.h"
#include "Tests/EnemiesTestFixture.h"
#include "Tests/ShootingAbilityFixture.h"

namespace EnemiesCombatTest
{
	struct FScene
	{
		AEnemyCharacter* Enemy = nullptr;
		ATargetDummy* Target = nullptr;

		/** Enemy at the origin facing +X, target dummy at TargetX. */
		FScene(const ShootingTest::FWorldFixture& F, const float TargetX = 1000.f)
		{
			EnemiesTest::SpawnFloor(F);
			Enemy = EnemiesTest::SpawnEnemy(F);
			Target = ShootingAbilityTest::SpawnDummy(F);
			Target->SetActorLocation(FVector(TargetX, 0.f, 100.f));
			F.Tick(0.2f); // land on the floor
		}
	};

	/** Static wall across the X axis at X. */
	void SpawnWall(const ShootingTest::FWorldFixture& F, const float X)
	{
		AActor* Wall = F.World->SpawnActor<AActor>();
		UBoxComponent* Box = NewObject<UBoxComponent>(Wall);
		Box->SetBoxExtent(FVector(20.f, 500.f, 300.f));
		Box->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
		Wall->SetRootComponent(Box);
		Box->RegisterComponent();
		Box->SetWorldLocation(FVector(X, 0.f, 300.f));
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesCombatClearLineTest, "TypicalShooter.Enemies.Combat.FiresWithLineOfSight", ShootingTest::Flags)
bool FEnemiesCombatClearLineTest::RunTest(const FString& Parameters)
{
	const ShootingTest::FWorldFixture F(true);
	const EnemiesCombatTest::FScene S(F);
	TestTrue(TEXT("Clear line to the target in range"), S.Enemy->CanShootAt(S.Target));
	S.Enemy->UpdateFiring(S.Target);
	TestTrue(TEXT("Fires at a visible target"), S.Enemy->IsFiring());
	S.Enemy->UpdateFiring(nullptr);
	TestFalse(TEXT("Stops without a target"), S.Enemy->IsFiring());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesCombatWallTest, "TypicalShooter.Enemies.Combat.WallBlocksFire", ShootingTest::Flags)
bool FEnemiesCombatWallTest::RunTest(const FString& Parameters)
{
	const ShootingTest::FWorldFixture F(true);
	const EnemiesCombatTest::FScene S(F);
	S.Enemy->UpdateFiring(S.Target);
	EnemiesCombatTest::SpawnWall(F, 500.f);
	TestFalse(TEXT("A wall blocks the line of sight"), S.Enemy->CanShootAt(S.Target));
	S.Enemy->UpdateFiring(S.Target);
	TestFalse(TEXT("Stops firing behind a wall"), S.Enemy->IsFiring());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesCombatAllyTest, "TypicalShooter.Enemies.Combat.AllyBlocksFire", ShootingTest::Flags)
bool FEnemiesCombatAllyTest::RunTest(const FString& Parameters)
{
	const ShootingTest::FWorldFixture F(true);
	const EnemiesCombatTest::FScene S(F);
	EnemiesTest::SpawnEnemy(F, FVector(500.f, 0.f, 0.f));
	F.Tick(0.2f);
	TestFalse(TEXT("Another enemy in the line blocks fire"), S.Enemy->CanShootAt(S.Target));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesCombatRangeTest, "TypicalShooter.Enemies.Combat.OutOfRange", ShootingTest::Flags)
bool FEnemiesCombatRangeTest::RunTest(const FString& Parameters)
{
	const ShootingTest::FWorldFixture F(true);
	const EnemiesCombatTest::FScene S(F, 6000.f); // fire end distance is 5000
	TestFalse(TEXT("No fire beyond the fire end distance"), S.Enemy->CanShootAt(S.Target));
	return true;
}

#endif
