// Power grid component: builds the grid from the items of its actor's inventory and recalculates it on every layout
// change (power grid R6b, R13a). Bare game world, server side.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "ItemsContainer.h"
#include "PowerGrid/PowerGridComponent.h"
#include "TypicalShooterCharacter.h"
#include "PowerGrid/PowerGridFragment.h"
#include "Tests/InventoryTestFixture.h"

namespace PowerGridComponentTest
{
	FInventoryItemManifest PowerManifest(const EPowerItemType Type, const int32 Faces = HexFaces::AllFaces, const int32 Power = 1)
	{
		FPowerGrid_ItemFragment Fragment;
		Fragment.ItemType = Type;
		Fragment.OpenFaces = Faces;
		Fragment.Source.Power = Power;
		FInventoryItemManifest Manifest;
		Manifest.Fragments.Add(TInstancedStruct<FItemFragment>::Make<FPowerGrid_ItemFragment>(Fragment));
		return Manifest;
	}

	UPowerGridComponent* AddPowerGrid(AActor* Actor)
	{
		UPowerGridComponent* Grid = NewObject<UPowerGridComponent>(Actor, TEXT("PowerGrid"));
		Grid->RegisterComponent();
		return Grid;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerGridComponentInitialTest, "TypicalShooter.PowerGrid.Component.FindsInventoryAndSolves", InventoryCoreTest::Flags)
bool FPowerGridComponentInitialTest::RunTest(const FString& Parameters)
{
	using namespace PowerGridComponentTest;
	InventoryCoreTest::FFixture F;
	UInventoryItem* Source = F.Inventory->AddItem(PowerManifest(EPowerItemType::Source, HexFaces::AllFaces, 3), FHhAxialCoord(0, 0));
	UInventoryItem* Connector = F.Inventory->AddItem(PowerManifest(EPowerItemType::Connector), FHhAxialCoord(1, 0));
	UInventoryItem* Plain = F.Inventory->AddItem(FInventoryItemManifest(), FHhAxialCoord(-1, 0));

	UPowerGridComponent* Grid = AddPowerGrid(F.Actor);
	TestTrue(TEXT("Finds the inventory of its actor"), Grid->GetInventory() == F.Inventory);
	TestEqual(TEXT("Items placed before registration are solved"), Grid->GetItemPower(Connector), 3);
	TestEqual(TEXT("Source reports its own power"), Grid->GetItemPower(Source), 3);
	TestTrue(TEXT("Connector is powered"), Grid->IsItemPowered(Connector));
	TestFalse(TEXT("Items without a power fragment are not in the grid"), Grid->IsInGrid(Plain));
	TestEqual(TEXT("... and have no power"), Grid->GetItemPower(Plain), 0);
	TestEqual(TEXT("Two grid nodes"), Grid->GetNodeItems().Num(), 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerGridComponentLayoutTest, "TypicalShooter.PowerGrid.Component.RecalculatesOnLayoutChange", InventoryCoreTest::Flags)
bool FPowerGridComponentLayoutTest::RunTest(const FString& Parameters)
{
	using namespace PowerGridComponentTest;
	InventoryCoreTest::FFixture F;
	UPowerGridComponent* Grid = AddPowerGrid(F.Actor);
	int32 NumChanges = 0;
	Grid->GetPowerGridChangedDelegate().AddLambda([&NumChanges](UPowerGridComponent*) { ++NumChanges; });

	const int32 LeftRight = HexFaces::Bit(EHexFace::Left) | HexFaces::Bit(EHexFace::Right);
	UInventoryItem* Source = F.Inventory->AddItem(PowerManifest(EPowerItemType::Source), FHhAxialCoord(0, 0));
	UInventoryItem* Connector = F.Inventory->AddItem(PowerManifest(EPowerItemType::Connector, LeftRight), FHhAxialCoord(1, 0));
	UInventoryItem* Receiver = F.Inventory->AddItem(PowerManifest(EPowerItemType::Receiver), FHhAxialCoord(2, 0));
	TestEqual(TEXT("Every addition recalculates"), NumChanges, 3);
	TestEqual(TEXT("Receiver fed through the connector"), Grid->GetItemPower(Receiver), 1);
	TestEqual(TEXT("Receiver is connected to the source and the connector"), Grid->GetConnectedItems(Receiver).Num(), 3);

	TestTrue(TEXT("Rotate the connector"), F.Inventory->RotateItem(Connector));
	TestEqual(TEXT("Rotation recalculates"), NumChanges, 4);
	TestEqual(TEXT("Rotated connector no longer faces the source (rotation is applied)"), Grid->GetItemPower(Connector), 0);
	TestEqual(TEXT("Receiver lost power"), Grid->GetItemPower(Receiver), 0);
	TestEqual(TEXT("Unfed receiver is connected only to itself"), Grid->GetConnectedItems(Receiver).Num(), 1);

	TestTrue(TEXT("Rotate back (5 more steps)"), F.Inventory->RotateItem(Connector, 5));
	TestEqual(TEXT("Receiver fed again"), Grid->GetItemPower(Receiver), 1);

	F.Inventory->RemoveItem(Source);
	TestEqual(TEXT("Removing the source cuts the power"), Grid->GetItemPower(Receiver), 0);
	TestFalse(TEXT("Removed item leaves the grid"), Grid->IsInGrid(Source));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerGridComponentAmmoSourceTest, "TypicalShooter.PowerGrid.Component.AmmoSourceWithoutClip", InventoryCoreTest::Flags)
bool FPowerGridComponentAmmoSourceTest::RunTest(const FString& Parameters)
{
	using namespace PowerGridComponentTest;
	InventoryCoreTest::FFixture F;
	UPowerGridComponent* Grid = AddPowerGrid(F.Actor);

	FInventoryItemManifest Manifest = PowerManifest(EPowerItemType::Source, HexFaces::AllFaces, 2);
	Manifest.Fragments[0].GetMutable<FPowerGrid_ItemFragment>().Source.AmmoType = FGameplayTag::RequestGameplayTag(TEXT("Ammo.Bullet"));
	UInventoryItem* Source = F.Inventory->AddItem(Manifest, FHhAxialCoord(0, 0));
	UInventoryItem* Connector = F.Inventory->AddItem(PowerManifest(EPowerItemType::Connector), FHhAxialCoord(1, 0));
	TestFalse(TEXT("A source needing ammo has an empty clip for now and does not work (R9d)"), Grid->IsItemPowered(Source));
	TestEqual(TEXT("... so it gives nothing"), Grid->GetItemPower(Connector), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerGridComponentActorsTest, "TypicalShooter.PowerGrid.Component.OnCharacterAndContainer", InventoryCoreTest::Flags)
bool FPowerGridComponentActorsTest::RunTest(const FString& Parameters)
{
	const ATypicalShooterCharacter* Character = GetDefault<ATypicalShooterCharacter>();
	TestNotNull(TEXT("Character has a power grid (R6b)"), Character->FindComponentByClass<UPowerGridComponent>());

	InventoryCoreTest::FFixture F;
	const AItemsContainer* Container = F.World->SpawnActor<AItemsContainer>();
	if (TestNotNull(TEXT("Container spawned"), Container))
	{
		const UPowerGridComponent* Grid = Container->FindComponentByClass<UPowerGridComponent>();
		if (TestNotNull(TEXT("Container has a power grid (R6b)"), Grid))
		{
			TestTrue(TEXT("... following the container's inventory"), Grid->GetInventory() == Container->FindComponentByClass<UInventoryComponent>());
		}
	}
	return true;
}

#endif
