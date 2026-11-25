// Tests for UAttackGameplayAbility: a shooter actor with an ASC fires at target dummies in a ticking world.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/ShootingAbilityFixture.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAttackSingleShotTest, "TypicalShooter.Shooting.Ability.SingleShot", ShootingTest::Flags)
bool FAttackSingleShotTest::RunTest(const FString& Parameters)
{
	using namespace ShootingAbilityTest;
	const ShootingTest::FWorldFixture F(true);
	const ATargetDummy* Dummy = SpawnDummy(F);
	const FShooter Shooter(F, EAttackSlot::Primary);
	Shooter.Setup(EAttackSlot::Primary, 1.f, 0.f, 0.f, 10.f);

	TestTrue(TEXT("Activates"), Shooter.Activate());
	TestEqual(TEXT("One projectile fired"), Shooter.Projectiles(), 1);
	F.Tick(0.2f);
	TestEqual(TEXT("The projectile hits the dummy"), Lost(Dummy), 10.f);
	TestFalse(TEXT("Ability ends after the burst"), Shooter.IsActive());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAttackBurstTest, "TypicalShooter.Shooting.Ability.Burst", ShootingTest::Flags)
bool FAttackBurstTest::RunTest(const FString& Parameters)
{
	using namespace ShootingAbilityTest;
	const ShootingTest::FWorldFixture F(true);
	const ATargetDummy* Dummy = SpawnDummy(F);
	const FShooter Shooter(F, EAttackSlot::Primary);
	Shooter.Setup(EAttackSlot::Primary, 3.f, 0.1f, 0.5f, 10.f);

	Shooter.Activate();
	TestEqual(TEXT("First shot is immediate"), Shooter.Projectiles(), 1);
	F.Tick(0.05f);
	TestEqual(TEXT("Next shot waits for the shot interval"), Shooter.Projectiles(), 1);
	F.Tick(0.25f);
	TestEqual(TEXT("Whole burst fired"), Shooter.Projectiles(), 3);
	TestTrue(TEXT("Ability waits out the burst interval"), Shooter.IsActive());
	F.Tick(0.6f);
	TestEqual(TEXT("All burst projectiles hit"), Lost(Dummy), 30.f);
	TestFalse(TEXT("Ability ends after the burst interval when not held"), Shooter.IsActive());
	TestEqual(TEXT("No extra burst without input"), Shooter.Projectiles(), 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAttackHoldTest, "TypicalShooter.Shooting.Ability.HoldRepeatsBursts", ShootingTest::Flags)
bool FAttackHoldTest::RunTest(const FString& Parameters)
{
	using namespace ShootingAbilityTest;
	const ShootingTest::FWorldFixture F(true);
	SpawnDummy(F);
	const FShooter Shooter(F, EAttackSlot::Primary);
	Shooter.Setup(EAttackSlot::Primary, 2.f, 0.1f, 0.3f, 1.f);

	Shooter.SetHeld(true);
	Shooter.Activate();
	F.Tick(0.65f); // bursts at 0 and 0.4 (= 0.1 + 0.3), next one at 0.8
	TestEqual(TEXT("Held input repeats bursts after the burst interval"), Shooter.Projectiles(), 4);

	Shooter.SetHeld(false);
	F.Tick(1.f);
	TestEqual(TEXT("Releasing stops further bursts"), Shooter.Projectiles(), 4);
	TestFalse(TEXT("Ability ended"), Shooter.IsActive());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAttackNoClipTest, "TypicalShooter.Shooting.Ability.NoClipLimit", ShootingTest::Flags)
bool FAttackNoClipTest::RunTest(const FString& Parameters)
{
	using namespace ShootingAbilityTest;
	const ShootingTest::FWorldFixture F(true);
	const ATargetDummy* Dummy = SpawnDummy(F);
	const FShooter Shooter(F, EAttackSlot::Primary);
	Shooter.Setup(EAttackSlot::Primary, 40.f, 0.f, 0.f, 1.f);

	Shooter.Activate();
	F.Tick(0.2f);
	TestEqual(TEXT("Attacks have no clip: the whole burst is fired (power grid R9a)"), Shooter.Projectiles(), 40);
	TestEqual(TEXT("Every shot hits"), Lost(Dummy), 40.f);
	TestFalse(TEXT("Ability ended"), Shooter.IsActive());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAttackPatternTest, "TypicalShooter.Shooting.Ability.PatternProjectiles", ShootingTest::Flags)
bool FAttackPatternTest::RunTest(const FString& Parameters)
{
	using namespace ShootingAbilityTest;
	const ShootingTest::FWorldFixture F(true);
	const ATargetDummy* Dummy = SpawnDummy(F);
	FShotPattern Pattern;
	Pattern.Offsets = { FVector2D(0.f, 0.f), FVector2D(1.f, 0.f), FVector2D(-1.f, 0.f) };
	const FShooter Shooter(F, EAttackSlot::Primary, Pattern);
	Shooter.Setup(EAttackSlot::Primary, 1.f, 0.f, 0.f, 5.f);

	Shooter.Activate();
	F.Tick(0.2f);
	TestEqual(TEXT("One shot spawns every pattern point"), Shooter.Projectiles(), 3);
	TestEqual(TEXT("Every pattern point hits"), Lost(Dummy), 15.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAttackAlternateSlotTest, "TypicalShooter.Shooting.Ability.AlternateSlot", ShootingTest::Flags)
bool FAttackAlternateSlotTest::RunTest(const FString& Parameters)
{
	using namespace ShootingAbilityTest;
	const ShootingTest::FWorldFixture F(true);
	const ATargetDummy* Dummy = SpawnDummy(F);
	const FShooter Shooter(F, EAttackSlot::Alternate);
	Shooter.Setup(EAttackSlot::Primary, 1.f, 0.f, 0.f, 10.f);
	Shooter.Setup(EAttackSlot::Alternate, 1.f, 0.f, 0.f, 3.f);

	Shooter.Activate();
	F.Tick(0.2f);
	TestEqual(TEXT("One alternate projectile"), Shooter.Projectiles(), 1);
	TestEqual(TEXT("Alternate damage applied, not primary"), Lost(Dummy), 3.f);
	return true;
}

#endif
