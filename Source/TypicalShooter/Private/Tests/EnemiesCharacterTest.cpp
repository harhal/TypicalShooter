// Tests for AEnemyCharacter: health, damage and its (weaker) projectile attack.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "AbilitySystemComponent.h"
#include "Shooting/AttackAttributeSet.h"
#include "Shooting/HealthComponent.h"
#include "Shooting/TargetDummy.h"
#include "Tests/EnemiesTestFixture.h"
#include "Tests/ShootingAbilityFixture.h"

namespace EnemiesCharacterTest
{
	float Stat(const AEnemyCharacter* Enemy, const EAttackAttribute Attribute)
	{
		return Enemy->GetAbilitySystemComponent()->GetNumericAttribute(UAttackAttributeSet::GetAttribute(EAttackSlot::Primary, Attribute));
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesCharacterDefaultsTest, "TypicalShooter.Enemies.Character.Defaults", ShootingTest::Flags)
bool FEnemiesCharacterDefaultsTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesCharacterTest;
	const ShootingTest::FWorldFixture F(true);
	EnemiesTest::SpawnFloor(F);
	const AEnemyCharacter* Enemy = EnemiesTest::SpawnEnemy(F);

	const UHealthComponent* Health = Enemy->GetHealth();
	TestTrue(TEXT("Enemy starts with full health"), Health && Health->GetHealth() > 0.f && Health->GetHealth() == Health->GetMaxHealth());
	TestFalse(TEXT("Enemy health does not refill"), Health && Health->bRefillWhenDepleted);

	TestTrue(TEXT("Attack base stats are applied: some physical damage"), Stat(Enemy, EAttackAttribute::DamagePhysical) > 0.f);
	TestTrue(TEXT("Weaker than the player's rifle (10 damage)"), Stat(Enemy, EAttackAttribute::DamagePhysical) < 10.f);
	TestTrue(TEXT("Slower projectiles than the player's rifle (8000)"), Stat(Enemy, EAttackAttribute::ProjectileSpeed) < 8000.f);
	TestTrue(TEXT("Pauses between bursts"), Stat(Enemy, EAttackAttribute::BurstInterval) >= 0.5f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesCharacterTakesDamageTest, "TypicalShooter.Enemies.Character.TakesDamage", ShootingTest::Flags)
bool FEnemiesCharacterTakesDamageTest::RunTest(const FString& Parameters)
{
	const ShootingTest::FWorldFixture F(true);
	EnemiesTest::SpawnFloor(F);
	AEnemyCharacter* Enemy = EnemiesTest::SpawnEnemy(F);
	const float Before = Enemy->GetHealth()->GetHealth();
	EnemiesTest::Hit(Enemy, 7.f);
	TestEqual(TEXT("Shot damage lowers the enemy's health"), Enemy->GetHealth()->GetHealth(), Before - 7.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesCharacterFiresTest, "TypicalShooter.Enemies.Character.FiresWhileFiring", ShootingTest::Flags)
bool FEnemiesCharacterFiresTest::RunTest(const FString& Parameters)
{
	const ShootingTest::FWorldFixture F(true);
	EnemiesTest::SpawnFloor(F);
	AEnemyCharacter* Enemy = EnemiesTest::SpawnEnemy(F);
	ATargetDummy* Dummy = ShootingAbilityTest::SpawnDummy(F);
	Dummy->SetActorLocation(FVector(800.f, 0.f, 100.f));
	const TSharedRef<int32> Projectiles = EnemiesTest::CountProjectiles(F);
	F.Tick(0.2f); // land on the floor

	TestFalse(TEXT("Not firing by default"), Enemy->IsFiring());
	Enemy->SetFiring(true);
	F.Tick(3.f);
	TestTrue(TEXT("Firing spawns projectiles"), *Projectiles > 0);
	TestTrue(TEXT("Projectiles hit the target in front"), Dummy->GetTotalDamageTaken() > 0.f);

	Enemy->SetFiring(false);
	F.Tick(3.f); // finish the current burst and interval
	const int32 AfterStop = *Projectiles;
	F.Tick(3.f);
	TestEqual(TEXT("No new projectiles after firing stopped"), *Projectiles, AfterStop);
	return true;
}

#endif
