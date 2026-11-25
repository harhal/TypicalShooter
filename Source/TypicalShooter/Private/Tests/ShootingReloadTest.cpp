// Tests for UReloadGameplayAbility and the attack's automatic reload on an empty clip (R11, R12).

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Shooting/ShootingTags.h"
#include "Tests/ShootingAbilityFixture.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FReloadManualTest, "TypicalShooter.Shooting.Ability.Reload", ShootingTest::Flags)
bool FReloadManualTest::RunTest(const FString& Parameters)
{
	using namespace ShootingAbilityTest;
	const ShootingTest::FWorldFixture F(true);
	SpawnDummy(F);
	const FShooter Shooter(F, EAttackSlot::Primary);
	Shooter.Setup(EAttackSlot::Primary, 3.f, 1.f, 0.f, 0.f, 1.f);
	Shooter.Set(EAttackSlot::Primary, EAttackAttribute::ClipSize, 10.f);
	Shooter.Set(EAttackSlot::Primary, EAttackAttribute::ReloadTime, 0.5f);

	TestTrue(TEXT("Reload starts"), Shooter.Reload());
	F.Tick(0.3f);
	TestEqual(TEXT("Clip is filled only after the reload time"), Shooter.Get(EAttackSlot::Primary, EAttackAttribute::Ammo), 3.f);
	TestFalse(TEXT("The attack can't fire while reloading"), Shooter.Activate());

	F.Tick(0.3f);
	TestEqual(TEXT("Clip filled"), Shooter.Get(EAttackSlot::Primary, EAttackAttribute::Ammo), 10.f);
	TestFalse(TEXT("Reload ended"), Shooter.IsReloading());
	TestTrue(TEXT("The attack fires again"), Shooter.Activate());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FReloadFullClipTest, "TypicalShooter.Shooting.Ability.ReloadSkipsFullClip", ShootingTest::Flags)
bool FReloadFullClipTest::RunTest(const FString& Parameters)
{
	using namespace ShootingAbilityTest;
	const ShootingTest::FWorldFixture F(true);
	const FShooter Shooter(F, EAttackSlot::Primary);
	Shooter.Setup(EAttackSlot::Primary, 30.f, 1.f, 0.f, 0.f, 1.f);
	Shooter.Set(EAttackSlot::Primary, EAttackAttribute::ReloadTime, 1.f);

	Shooter.Reload();
	TestFalse(TEXT("A full clip is not reloaded"), Shooter.IsReloading());
	TestTrue(TEXT("The attack is not blocked"), Shooter.Activate());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FReloadAutoTest, "TypicalShooter.Shooting.Ability.AutoReloadWhenEmpty", ShootingTest::Flags)
bool FReloadAutoTest::RunTest(const FString& Parameters)
{
	using namespace ShootingAbilityTest;
	const ShootingTest::FWorldFixture F(true);
	SpawnDummy(F);
	const FShooter Shooter(F, EAttackSlot::Alternate);
	Shooter.Setup(EAttackSlot::Alternate, 1.f, 3.f, 0.05f, 0.f, 1.f);
	Shooter.Set(EAttackSlot::Alternate, EAttackAttribute::ReloadTime, 0.2f);

	Shooter.Activate();
	F.Tick(0.1f);
	TestTrue(TEXT("Running out of rounds starts a reload"), Shooter.IsReloading());
	F.Tick(0.3f);
	TestEqual(TEXT("Clip refilled"), Shooter.Get(EAttackSlot::Alternate, EAttackAttribute::Ammo), 30.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FReloadFromReserveTest, "TypicalShooter.Shooting.Ability.ReloadFromReserve", ShootingTest::Flags)
bool FReloadFromReserveTest::RunTest(const FString& Parameters)
{
	using namespace ShootingAbilityTest;
	const ShootingTest::FWorldFixture F(true);
	const FShooter Shooter(F, EAttackSlot::Primary);
	Shooter.Setup(EAttackSlot::Primary, 3.f, 1.f, 0.f, 0.f, 1.f);
	Shooter.Set(EAttackSlot::Primary, EAttackAttribute::ClipSize, 10.f);
	Shooter.SetAmmoType(ShootingTags::Ammo_Battery);

	Shooter.SetReserve(ShootingTags::Ammo_Battery, 100.f);
	Shooter.SetReserve(ShootingTags::Ammo_Bullet, 50.f);
	Shooter.Reload();
	TestEqual(TEXT("Clip filled from the reserve"), Shooter.Get(EAttackSlot::Primary, EAttackAttribute::Ammo), 10.f);
	TestEqual(TEXT("Only the missing rounds leave the reserve"), Shooter.Reserve(ShootingTags::Ammo_Battery), 93.f);
	TestEqual(TEXT("Other ammo kinds are untouched"), Shooter.Reserve(ShootingTags::Ammo_Bullet), 50.f);

	Shooter.Set(EAttackSlot::Primary, EAttackAttribute::Ammo, 2.f);
	Shooter.SetReserve(ShootingTags::Ammo_Battery, 5.f);
	Shooter.Reload();
	TestEqual(TEXT("A short reserve loads what it has"), Shooter.Get(EAttackSlot::Primary, EAttackAttribute::Ammo), 7.f);
	TestEqual(TEXT("Reserve emptied"), Shooter.Reserve(ShootingTags::Ammo_Battery), 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FReloadEmptyReserveTest, "TypicalShooter.Shooting.Ability.NoReloadWithoutReserve", ShootingTest::Flags)
bool FReloadEmptyReserveTest::RunTest(const FString& Parameters)
{
	using namespace ShootingAbilityTest;
	const ShootingTest::FWorldFixture F(true);
	const FShooter Shooter(F, EAttackSlot::Alternate);
	Shooter.Setup(EAttackSlot::Alternate, 0.f, 1.f, 0.f, 0.f, 1.f);
	Shooter.Set(EAttackSlot::Alternate, EAttackAttribute::ReloadTime, 1.f);
	Shooter.SetAmmoType(ShootingTags::Ammo_Grenade);
	Shooter.SetReserve(ShootingTags::Ammo_Grenade, 0.f);

	Shooter.Reload();
	TestFalse(TEXT("No reload without reserve"), Shooter.IsReloading());
	Shooter.Activate();
	TestFalse(TEXT("Empty press with no reserve does not start a reload"), Shooter.IsReloading());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FReloadOnEmptyPressTest, "TypicalShooter.Shooting.Ability.EmptyPressReloads", ShootingTest::Flags)
bool FReloadOnEmptyPressTest::RunTest(const FString& Parameters)
{
	using namespace ShootingAbilityTest;
	const ShootingTest::FWorldFixture F(true);
	const FShooter Shooter(F, EAttackSlot::Primary);
	Shooter.Setup(EAttackSlot::Primary, 0.f, 1.f, 0.f, 0.f, 1.f);
	Shooter.Set(EAttackSlot::Primary, EAttackAttribute::ReloadTime, 0.2f);

	Shooter.Activate();
	TestTrue(TEXT("Firing with an empty clip starts a reload"), Shooter.IsReloading());
	TestFalse(TEXT("The attack itself ends"), Shooter.IsActive());
	return true;
}

#endif
