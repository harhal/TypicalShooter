// Source clips (power grid R9, R9d): a source needing ammo works only with a clip; entering the character's inventory
// it reserves up to its capacity from the reserve, leaving it returns the rest. A clip is never carried along.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "PowerGrid/SourceAmmoComponent.h"
#include "Tests/InventoryTestFixture.h"
#include "Shooting/AmmoAttributeSet.h"
#include "Shooting/ShootingTags.h"
#include "Tests/PowerGridTestFixture.h"
#include "TypicalShooterCharacter.h"

namespace PowerGridSourceAmmoTest
{
	FInventoryItemManifest AmmoSource(const int32 Capacity, const int32 Power = 1)
	{
		FInventoryItemManifest Result = PowerGridTest::Manifest(EPowerItemType::Source);
		FPowerSourceParams& Source = Result.Fragments[0].GetMutable<FPowerGrid_ItemFragment>().Source;
		Source.AmmoType = ShootingTags::Ammo_Bullet;
		Source.Capacity = Capacity;
		Source.Power = Power;
		return Result;
	}

	struct FAmmoFixture : PowerGridTest::FArmed
	{
		UPowerGridComponent* Grid = nullptr;

		FAmmoFixture(const ShootingTest::FWorldFixture& F, const float Bullets) : FArmed(F)
		{
			Asc->AddSpawnedAttribute(NewObject<UAmmoAttributeSet>(Character));
			SetReserve(Bullets);
			NewObject<USourceAmmoComponent>(Character)->RegisterComponent();
			Grid = Character->FindComponentByClass<UPowerGridComponent>();
		}

		float Reserve() const { return Asc->GetNumericAttribute(UAmmoAttributeSet::GetReserveAttribute(ShootingTags::Ammo_Bullet)); }
		void SetReserve(const float Value) const { Asc->SetNumericAttributeBase(UAmmoAttributeSet::GetReserveAttribute(ShootingTags::Ammo_Bullet), Value); }
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerGridClipStateTest, "TypicalShooter.PowerGrid.SourceAmmo.ClipDrivesGrid", ShootingTest::Flags)
bool FPowerGridClipStateTest::RunTest(const FString& Parameters)
{
	using namespace PowerGridSourceAmmoTest;
	InventoryCoreTest::FFixture F;
	UPowerGridComponent* Grid = NewObject<UPowerGridComponent>(F.Actor);
	Grid->RegisterComponent();

	UInventoryItem* Source = F.Inventory->AddItem(AmmoSource(4), FHhAxialCoord(0, 0));
	TestFalse(TEXT("No clip: the source does not work (R9)"), Grid->IsItemPowered(Source));
	Source->SetClip(1);
	TestTrue(TEXT("A clip change re-solves the grid: the source works"), Grid->IsItemPowered(Source));
	Source->SetClip(0);
	TestFalse(TEXT("Empty again: off"), Grid->IsItemPowered(Source));
	Source->SetClip(-3);
	TestEqual(TEXT("The clip is never negative"), Source->GetClip(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerGridClipReserveTest, "TypicalShooter.PowerGrid.SourceAmmo.ReserveAndReturn", ShootingTest::Flags)
bool FPowerGridClipReserveTest::RunTest(const FString& Parameters)
{
	using namespace PowerGridSourceAmmoTest;
	const ShootingTest::FWorldFixture F(true);
	const FAmmoFixture A(F, 10.f);

	UInventoryItem* Small = A.Inventory->AddItem(AmmoSource(4), FHhAxialCoord(0, 0));
	TestEqual(TEXT("Entering, the source reserves up to its capacity (R9d)"), Small->GetClip(), 4);
	TestEqual(TEXT("... from the reserve"), A.Reserve(), 6.f);
	TestTrue(TEXT("With a clip it works"), A.Grid->IsItemPowered(Small));

	UInventoryItem* Big = A.Inventory->AddItem(AmmoSource(8), FHhAxialCoord(1, 1));
	TestEqual(TEXT("Only what the reserve has"), Big->GetClip(), 6);
	TestEqual(TEXT("Reserve emptied"), A.Reserve(), 0.f);

	A.Inventory->RemoveItem(Small);
	TestEqual(TEXT("Leaving, the source returns its clip (R9d)"), A.Reserve(), 4.f);

	UInventoryComponent* Container = NewObject<UInventoryComponent>(F.World->SpawnActor<AActor>());
	Container->RegisterComponent();
	A.Inventory->TransferItem(Big, Container, FHhAxialCoord(0, 0));
	TestEqual(TEXT("Transfer out returns the clip"), A.Reserve(), 10.f);
	UInventoryItem* Stored = Container->GetItemAt(FHhAxialCoord(0, 0));
	TestEqual(TEXT("In a container without a reserve the clip is empty (R9d)"), Stored ? Stored->GetClip() : -1, 0);

	Container->TransferItem(Stored, A.Inventory, FHhAxialCoord(0, 0));
	UInventoryItem* Back = A.Inventory->GetItemAt(FHhAxialCoord(0, 0));
	TestEqual(TEXT("Back in, it reserves again"), Back ? Back->GetClip() : -1, 8);
	TestEqual(TEXT("Reserve after the round trip"), A.Reserve(), 2.f);

	A.Inventory->AddItem(PowerGridTest::Manifest(EPowerItemType::Source), FHhAxialCoord(-1, 0));
	TestEqual(TEXT("A source without ammo reserves nothing"), A.Reserve(), 2.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerGridSourceAmmoOnCharacterTest, "TypicalShooter.PowerGrid.SourceAmmo.OnCharacter", ShootingTest::Flags)
bool FPowerGridSourceAmmoOnCharacterTest::RunTest(const FString& Parameters)
{
	TestNotNull(TEXT("Character manages source clips"), GetDefault<ATypicalShooterCharacter>()->FindComponentByClass<USourceAmmoComponent>());
	return true;
}

#endif
