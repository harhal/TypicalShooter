// Tests for attack modifiers from inventory items (R16) and shot pattern replacement (R5).

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "InventoryComponent.h"
#include "InventoryItemManifest.h"
#include "Shooting/AttackModifierFragment.h"
#include "Shooting/AttackModifiersComponent.h"
#include "Shooting/ShootingTags.h"
#include "Tests/ShootingAbilityFixture.h"
#include "Tests/ShootingTestEffects.h"

namespace ShootingItemModifiersTest
{
	/** The shooter's actor also gets an inventory and the modifiers component. */
	struct FArmedShooter : ShootingAbilityTest::FShooter
	{
		UInventoryComponent* Inventory = nullptr;
		UAttackModifiersComponent* Modifiers = nullptr;

		FArmedShooter(const ShootingTest::FWorldFixture& F, const EAttackSlot Slot) : FShooter(F, Slot)
		{
			AActor* Actor = Asc->GetOwner();
			Inventory = NewObject<UInventoryComponent>(Actor);
			Inventory->RegisterComponent();
			Modifiers = NewObject<UAttackModifiersComponent>(Actor);
			Modifiers->RegisterComponent();
		}
	};

	FInventoryItemManifest ModifierManifest(const TSubclassOf<UGameplayEffect> Effect)
	{
		FAttackModifier_ItemFragment Fragment;
		Fragment.Effect = Effect;
		FInventoryItemManifest Manifest;
		Manifest.Fragments.Add(TInstancedStruct<FItemFragment>::Make<FAttackModifier_ItemFragment>(Fragment));
		return Manifest;
	}

	FInventoryItemManifest PatternManifest(const EAttackSlot Slot, const int32 NumPoints)
	{
		FAttackModifier_ItemFragment Fragment;
		Fragment.bReplacePattern = true;
		Fragment.PatternSlot = Slot;
		Fragment.Pattern.Offsets.Reset();
		for (int32 Index = 0; Index < NumPoints; ++Index)
		{
			Fragment.Pattern.Offsets.Add(FVector2D(Index - (NumPoints - 1) * 0.5f, 0.f));
		}
		FInventoryItemManifest Manifest;
		Manifest.Fragments.Add(TInstancedStruct<FItemFragment>::Make<FAttackModifier_ItemFragment>(Fragment));
		return Manifest;
	}
}

namespace ShootingItemModifiersTest
{
	/** Item adding Bonus acid damage to Slot's shots, each shot costing Cost batteries. */
	FInventoryItemManifest AmmoGatedManifest(const EAttackSlot Slot, const int32 Cost, const float Bonus)
	{
		FAmmoGatedShotModifier Modifier;
		Modifier.Slot = Slot;
		Modifier.AmmoType = ShootingTags::Ammo_Battery;
		Modifier.CostPerShot = Cost;
		Modifier.Bonuses.Add({ EAttackAttribute::DamageAcid, Bonus });

		FAttackModifier_ItemFragment Fragment;
		Fragment.ShotModifiers.Add(Modifier);
		FInventoryItemManifest Manifest;
		Manifest.Fragments.Add(TInstancedStruct<FItemFragment>::Make<FAttackModifier_ItemFragment>(Fragment));
		return Manifest;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAmmoGatedModifierTest, "TypicalShooter.Shooting.Ability.AmmoGatedModifier", ShootingTest::Flags)
bool FAmmoGatedModifierTest::RunTest(const FString& Parameters)
{
	using namespace ShootingItemModifiersTest;
	const ShootingTest::FWorldFixture F(true);
	const ATargetDummy* Dummy = ShootingAbilityTest::SpawnDummy(F);
	const FArmedShooter Shooter(F, EAttackSlot::Primary);
	Shooter.Setup(EAttackSlot::Primary, 10.f, 1.f, 0.f, 0.f, 10.f);
	Shooter.SetReserve(ShootingTags::Ammo_Battery, 3.f);
	Shooter.Inventory->AddItem(AmmoGatedManifest(EAttackSlot::Primary, 2, 7.f), FHhAxialCoord(0, 0));
	Shooter.Inventory->AddItem(AmmoGatedManifest(EAttackSlot::Alternate, 1, 100.f), FHhAxialCoord(1, 0));

	Shooter.Activate();
	F.Tick(0.2f);
	TestEqual(TEXT("Paid modifier adds its bonus to the shot"), ShootingAbilityTest::Lost(Dummy), 17.f);
	TestEqual(TEXT("The cost is taken from the reserve"), Shooter.Reserve(ShootingTags::Ammo_Battery), 1.f);

	Shooter.Activate();
	F.Tick(0.2f);
	TestEqual(TEXT("Without enough reserve the modifier is skipped, the shot still fires"), ShootingAbilityTest::Lost(Dummy), 27.f);
	TestEqual(TEXT("Nothing is taken when the cost can't be paid"), Shooter.Reserve(ShootingTags::Ammo_Battery), 1.f);
	TestEqual(TEXT("Clip rounds spent as usual"), Shooter.Get(EAttackSlot::Primary, EAttackAttribute::Ammo), 8.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FItemModifierEffectTest, "TypicalShooter.Shooting.ItemModifiers.EffectWhileHeld", ShootingTest::Flags)
bool FItemModifierEffectTest::RunTest(const FString& Parameters)
{
	using namespace ShootingItemModifiersTest;
	const ShootingTest::FWorldFixture F(true);
	const FArmedShooter Shooter(F, EAttackSlot::Primary);
	Shooter.Set(EAttackSlot::Primary, EAttackAttribute::DamagePhysical, 10.f);

	UInventoryItem* Item = Shooter.Inventory->AddItem(ModifierManifest(UTestPrimaryDamageBonusEffect::StaticClass()), FHhAxialCoord(0, 0));
	TestEqual(TEXT("Held item modifies the attack attribute"), Shooter.Get(EAttackSlot::Primary, EAttackAttribute::DamagePhysical), 15.f);
	TestEqual(TEXT("Other slot untouched"), Shooter.Get(EAttackSlot::Alternate, EAttackAttribute::DamagePhysical), 0.f);

	Shooter.Inventory->AddItem(ModifierManifest(UTestPrimaryDamageBonusEffect::StaticClass()), FHhAxialCoord(1, 0));
	TestEqual(TEXT("Modifiers of several items stack"), Shooter.Get(EAttackSlot::Primary, EAttackAttribute::DamagePhysical), 20.f);

	Shooter.Inventory->RemoveItem(Item);
	TestEqual(TEXT("Removing an item removes its effect"), Shooter.Get(EAttackSlot::Primary, EAttackAttribute::DamagePhysical), 15.f);

	UInventoryComponent* Other = NewObject<UInventoryComponent>(F.World->SpawnActor<AActor>());
	Other->RegisterComponent();
	Shooter.Inventory->TransferItemToFreeLocation(Shooter.Inventory->GetPlacedItems()[0].Key, Other);
	TestEqual(TEXT("Moving the item to another inventory removes its effect"), Shooter.Get(EAttackSlot::Primary, EAttackAttribute::DamagePhysical), 10.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FItemModifierPatternTest, "TypicalShooter.Shooting.ItemModifiers.PatternReplacement", ShootingTest::Flags)
bool FItemModifierPatternTest::RunTest(const FString& Parameters)
{
	using namespace ShootingItemModifiersTest;
	const ShootingTest::FWorldFixture F(true);
	const FArmedShooter Shooter(F, EAttackSlot::Primary);

	TestNull(TEXT("No override without items"), Shooter.Modifiers->FindPatternOverride(EAttackSlot::Primary));
	Shooter.Inventory->AddItem(PatternManifest(EAttackSlot::Primary, 3), FHhAxialCoord(0, 0));
	UInventoryItem* Second = Shooter.Inventory->AddItem(PatternManifest(EAttackSlot::Primary, 5), FHhAxialCoord(1, 0));
	Shooter.Inventory->AddItem(PatternManifest(EAttackSlot::Alternate, 2), FHhAxialCoord(0, 1));

	const FShotPattern* Override = Shooter.Modifiers->FindPatternOverride(EAttackSlot::Primary);
	TestTrue(TEXT("The last added item's pattern wins"), Override && Override->GetNumProjectiles() == 5);
	Shooter.Inventory->RemoveItem(Second);
	Override = Shooter.Modifiers->FindPatternOverride(EAttackSlot::Primary);
	TestTrue(TEXT("Removing it restores the previous item's pattern"), Override && Override->GetNumProjectiles() == 3);
	const FShotPattern* Alternate = Shooter.Modifiers->FindPatternOverride(EAttackSlot::Alternate);
	TestTrue(TEXT("Overrides are per slot"), Alternate && Alternate->GetNumProjectiles() == 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FItemModifierFireTest, "TypicalShooter.Shooting.ItemModifiers.AttackUsesModifiers", ShootingTest::Flags)
bool FItemModifierFireTest::RunTest(const FString& Parameters)
{
	using namespace ShootingItemModifiersTest;
	const ShootingTest::FWorldFixture F(true);
	const ATargetDummy* Dummy = ShootingAbilityTest::SpawnDummy(F);
	const FArmedShooter Shooter(F, EAttackSlot::Primary);
	Shooter.Setup(EAttackSlot::Primary, 10.f, 1.f, 0.f, 0.f, 10.f);
	Shooter.Inventory->AddItem(ModifierManifest(UTestPrimaryDamageBonusEffect::StaticClass()), FHhAxialCoord(0, 0));
	Shooter.Inventory->AddItem(PatternManifest(EAttackSlot::Primary, 3), FHhAxialCoord(1, 0));

	Shooter.Activate();
	F.Tick(0.2f);
	TestEqual(TEXT("Three projectiles of the replaced pattern, 15 damage each"), ShootingAbilityTest::Lost(Dummy), 45.f);
	return true;
}

#endif
