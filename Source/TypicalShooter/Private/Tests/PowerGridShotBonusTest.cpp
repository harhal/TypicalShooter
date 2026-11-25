// Shots through the power grid (R14b, replacing the old item attack modifiers): item bonuses by power thresholds reach
// the shot, an ammo source feeding a booster makes a bonus that lasts while its clip does, and the shot pattern comes
// from the receiver's ability.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "PowerGrid/PowerEffectsComponent.h"
#include "PowerGrid/PowerEffectsFragment.h"
#include "PowerGrid/SourceAmmoComponent.h"
#include "Shooting/AmmoAttributeSet.h"
#include "Shooting/DefaultAttackSetup.h"
#include "Shooting/ShootingTags.h"
#include "Tests/PowerGridTestFixture.h"

namespace PowerGridShotBonusTest
{
	struct FShooter : PowerGridTest::FArmed
	{
		explicit FShooter(const ShootingTest::FWorldFixture& F) : FArmed(F)
		{
			Asc->AddSpawnedAttribute(NewObject<UAmmoAttributeSet>(Character));
			Asc->SetNumericAttributeBase(UAmmoAttributeSet::GetReserveAttribute(ShootingTags::Ammo_Battery), 1.f);
			NewObject<USourceAmmoComponent>(Character)->RegisterComponent();
			NewObject<UPowerEffectsComponent>(Character)->RegisterComponent();
		}

		/** Fires the primary attack once; the damage of the shot's first projectile of Type. */
		float Shoot(const ShootingTest::FWorldFixture& F, const FGameplayTag& Type)
		{
			Projectiles.Reset();
			const FGameplayAbilitySpec* Spec = FindSpec(EAttackInputID::PrimaryAttack);
			const bool bFired = Spec && Asc->TryActivateAbility(Spec->Handle);
			const float Damage = bFired && Projectiles.Num() > 0 && Projectiles[0].IsValid() ? Projectiles[0]->GetRemainingDamage().Get(Type) : -1.f;
			F.Tick(0.2f);
			return Damage;
		}
	};

	FInventoryItemManifest Booster()
	{
		FPowerEffects_ItemFragment Effects;
		FPowerThresholdEffect& Physical = Effects.Effects.AddDefaulted_GetRef();
		Physical.Threshold = 1;
		Physical.AttackBonuses.Add({ EAttackAttribute::DamagePhysical, 5.f });
		FPowerThresholdEffect& Acid = Effects.Effects.AddDefaulted_GetRef();
		Acid.Threshold = 2;
		Acid.AttackBonuses.Add({ EAttackAttribute::DamageAcid, 7.f });
		FInventoryItemManifest Result = PowerGridTest::Manifest(EPowerItemType::Connector);
		Result.Fragments.Add(TInstancedStruct<FItemFragment>::Make<FPowerEffects_ItemFragment>(Effects));
		return Result;
	}

	FInventoryItemManifest Battery()
	{
		FInventoryItemManifest Result = PowerGridTest::Manifest(EPowerItemType::Source);
		FPowerSourceParams& Source = Result.Fragments[0].GetMutable<FPowerGrid_ItemFragment>().Source;
		Source.AmmoType = ShootingTags::Ammo_Battery;
		Source.Capacity = 1;
		return Result;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerGridShotBonusTest, "TypicalShooter.PowerGrid.Effects.ShotUsesBonuses", ShootingTest::Flags)
bool FPowerGridShotBonusTest::RunTest(const FString& Parameters)
{
	using namespace PowerGridShotBonusTest;
	const ShootingTest::FWorldFixture F(true);
	FShooter A(F);
	const TArray<FAttackStatBonus> Base = { { EAttackAttribute::DamagePhysical, 10.f }, { EAttackAttribute::BurstInterval, 0.1f } };

	// Core -> receiver directly; the booster (1,-1) touches the core and the receiver, the battery (2,-2) only the booster.
	A.Inventory->AddItem(PowerGridTest::Manifest(EPowerItemType::Source), FHhAxialCoord(0, 0));
	A.Inventory->AddItem(PowerGridTest::Manifest(EPowerItemType::Receiver, Base), ReceiverSlots::GetLocation(EReceiverSlot::Primary));
	A.Inventory->AddItem(Booster(), FHhAxialCoord(1, -1));
	A.Inventory->AddItem(Battery(), FHhAxialCoord(2, -2));
	F.Tick(1.f / 60.f);

	TestEqual(TEXT("The shot carries the receiver base plus the booster's bonus"), A.Shoot(F, ShootingTags::Damage_Type_Physical), 15.f);
	TestEqual(TEXT("Next shot: the battery spent its clip on the previous one, the power-2 step is off"),
		A.Shoot(F, ShootingTags::Damage_Type_Acid), 0.f);

	A.Projectiles.Reset();
	A.Asc->SetNumericAttributeBase(UAmmoAttributeSet::GetReserveAttribute(ShootingTags::Ammo_Battery), 5.f);
	F.Tick(1.1f);
	TestEqual(TEXT("A recharged battery brings the ammo-paid bonus back (R14b)"), A.Shoot(F, ShootingTags::Damage_Type_Acid), 7.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerGridReceiverPatternTest, "TypicalShooter.PowerGrid.Receivers.AbilityPattern", ShootingTest::Flags)
bool FPowerGridReceiverPatternTest::RunTest(const FString& Parameters)
{
	using namespace PowerGridShotBonusTest;
	const ShootingTest::FWorldFixture F(true);
	FShooter A(F);
	FInventoryItemManifest Shotgun = PowerGridTest::Manifest(EPowerItemType::Receiver);
	Shotgun.Fragments[0].GetMutable<FPowerGrid_ItemFragment>().Receiver.Ability = UAlternateAttackAbility::StaticClass();
	A.Inventory->AddItem(PowerGridTest::Manifest(EPowerItemType::Source), FHhAxialCoord(0, 0));
	A.Inventory->AddItem(Shotgun, ReceiverSlots::GetLocation(EReceiverSlot::Primary));
	F.Tick(1.f / 60.f);

	A.Shoot(F, ShootingTags::Damage_Type_Physical);
	TestEqual(TEXT("The pattern is the receiver ability's own, whatever the slot (R14b)"), A.Projectiles.Num(), 7);
	return true;
}

#endif
