// Tests for the HUD ammo text: "clip / reserve" per attack slot.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Shooting/AmmoDisplay.h"
#include "Shooting/ShootingTags.h"
#include "Tests/ShootingAbilityFixture.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAmmoDisplayTextTest, "TypicalShooter.Shooting.UI.AmmoText", ShootingTest::Flags)
bool FAmmoDisplayTextTest::RunTest(const FString& Parameters)
{
	using namespace ShootingAbilityTest;
	const ShootingTest::FWorldFixture F(true);
	const FShooter Shooter(F, EAttackSlot::Primary);
	Shooter.Setup(EAttackSlot::Primary, 7.f, 1.f, 0.f, 0.f, 1.f);

	TestEqual(TEXT("Unlimited reserve"), AmmoDisplay::GetSlotText(*Shooter.Asc, EAttackSlot::Primary).ToString(), FString(TEXT("7 / ∞")));

	Shooter.SetAmmoType(ShootingTags::Ammo_Battery);
	Shooter.SetReserve(ShootingTags::Ammo_Battery, 40.f);
	TestEqual(TEXT("Clip / reserve of the slot's ammo kind"), AmmoDisplay::GetSlotText(*Shooter.Asc, EAttackSlot::Primary).ToString(), FString(TEXT("7 / 40")));

	Shooter.Set(EAttackSlot::Primary, EAttackAttribute::ReloadTime, 1.f);
	Shooter.Reload();
	TestTrue(TEXT("Reload is shown"), AmmoDisplay::GetSlotText(*Shooter.Asc, EAttackSlot::Primary).ToString().Contains(TEXT("Reloading")));
	TestEqual(TEXT("Slot without a reload ability has an unlimited reserve"),
		AmmoDisplay::GetSlotText(*Shooter.Asc, EAttackSlot::Alternate).ToString(), FString(TEXT("0 / ∞")));
	return true;
}

#endif
