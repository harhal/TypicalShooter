// Tests for the enemy's death at zero health.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/CapsuleComponent.h"
#include "Tests/EnemiesTestFixture.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesDiesAtZeroHealthTest, "TypicalShooter.Enemies.Character.DiesAtZeroHealth", ShootingTest::Flags)
bool FEnemiesDiesAtZeroHealthTest::RunTest(const FString& Parameters)
{
	const ShootingTest::FWorldFixture F(true);
	EnemiesTest::SpawnFloor(F);
	AEnemyCharacter* Enemy = EnemiesTest::SpawnEnemy(F);
	TSharedRef<int32> Deaths = MakeShared<int32>(0);
	Enemy->GetDiedDelegate().AddLambda([Deaths](AEnemyCharacter*) { ++*Deaths; });

	EnemiesTest::Hit(Enemy, 10.f);
	TestFalse(TEXT("Alive while health is left"), Enemy->IsDead());

	EnemiesTest::Hit(Enemy, 1000.f);
	TestTrue(TEXT("Dead at zero health"), Enemy->IsDead());
	TestEqual(TEXT("Death reported once"), *Deaths, 1);
	TestEqual(TEXT("No collision after death"), Enemy->GetCapsuleComponent()->GetCollisionEnabled(), ECollisionEnabled::NoCollision);

	EnemiesTest::Hit(Enemy, 10.f);
	TestEqual(TEXT("A dead enemy does not die again"), *Deaths, 1);

	const TWeakObjectPtr<AEnemyCharacter> Weak = Enemy;
	F.Tick(1.f);
	TestFalse(TEXT("The body disappears"), Weak.IsValid());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesDeadStopsFiringTest, "TypicalShooter.Enemies.Character.DiesAndStopsFiring", ShootingTest::Flags)
bool FEnemiesDeadStopsFiringTest::RunTest(const FString& Parameters)
{
	const ShootingTest::FWorldFixture F(true);
	EnemiesTest::SpawnFloor(F);
	AEnemyCharacter* Enemy = EnemiesTest::SpawnEnemy(F);
	const TSharedRef<int32> Projectiles = EnemiesTest::CountProjectiles(F);
	F.Tick(0.2f);
	Enemy->SetFiring(true);
	F.Tick(0.1f);
	TestTrue(TEXT("Fired before death"), *Projectiles > 0);

	EnemiesTest::Hit(Enemy, 1000.f);
	TestFalse(TEXT("Not firing when dead"), Enemy->IsFiring());
	const int32 AtDeath = *Projectiles;
	Enemy->SetFiring(true);
	F.Tick(3.f);
	TestEqual(TEXT("No shots after death"), *Projectiles, AtDeath);
	return true;
}

#endif
