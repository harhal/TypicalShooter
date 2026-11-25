// Tests for the HUD ammo text: the character's reserve per ammo kind (attacks have no clips, power grid R9a).

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

	TestEqual(TEXT("No reserve: nothing to show"), AmmoDisplay::GetReserveText(*Shooter.Asc).ToString(), FString());

	Shooter.SetReserve(ShootingTags::Ammo_Particle, 24.f);
	Shooter.SetReserve(ShootingTags::Ammo_Bullet, 120.f);
	TestEqual(TEXT("One line per ammo kind with a reserve, in tag order"),
		AmmoDisplay::GetReserveText(*Shooter.Asc).ToString(), FString(TEXT("Bullet: 120\nParticle: 24")));

	Shooter.SetReserve(ShootingTags::Ammo_Bullet, 0.f);
	TestEqual(TEXT("Empty kinds are hidden"), AmmoDisplay::GetReserveText(*Shooter.Asc).ToString(), FString(TEXT("Particle: 24")));
	return true;
}

#endif
