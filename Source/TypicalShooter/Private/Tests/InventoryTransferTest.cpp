// Inventory grid bounds and moving items within / between inventories (server-side logic, no PIE).

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "ItemsContainer.h"
#include "Tests/InventoryTestFixture.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryBoundsTest, "TypicalShooter.Inventory.Core.GridBounds", InventoryCoreTest::Flags)
bool FInventoryBoundsTest::RunTest(const FString& Parameters)
{
	InventoryCoreTest::FFixture F;
	F.Inventory->SetGridRadius(1);
	TestEqual(TEXT("Radius is stored"), F.Inventory->GetGridRadius(), 1);

	TestTrue(TEXT("Centre is in bounds"), F.Inventory->IsLocationInBounds(FHhAxialCoord(0, 0)));
	TestTrue(TEXT("First ring is in bounds"), F.Inventory->IsLocationInBounds(FHhAxialCoord(-1, 1)));
	TestFalse(TEXT("Second ring is out of bounds"), F.Inventory->IsLocationInBounds(FHhAxialCoord(2, -1)));
	TestFalse(TEXT("Second ring (other side) is out of bounds"), F.Inventory->IsLocationInBounds(FHhAxialCoord(-1, -1)));

	TestNull(TEXT("Cannot add outside the grid"), F.Add(FHhAxialCoord(2, 0)));
	UInventoryItem* Item = F.Add(FHhAxialCoord(1, 0));
	TestNotNull(TEXT("Can add on the edge of the grid"), Item);
	TestFalse(TEXT("Cannot move outside the grid"), F.Inventory->MoveItem(Item, FHhAxialCoord(1, 1)));
	TestTrue(TEXT("Item stays in place"), F.LocationOf(Item) == TOptional<FHhAxialCoord>(FHhAxialCoord(1, 0)));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryFreeLocationTest, "TypicalShooter.Inventory.Core.FindFreeLocation", InventoryCoreTest::Flags)
bool FInventoryFreeLocationTest::RunTest(const FString& Parameters)
{
	InventoryCoreTest::FFixture F;
	F.Inventory->SetGridRadius(1);

	TestTrue(TEXT("Empty inventory: the centre is free"), F.Inventory->FindFreeLocation() == TOptional<FHhAxialCoord>(FHhAxialCoord(0, 0)));

	TSet<FHhAxialCoord> Used;
	for (int32 i = 0; i < 7; ++i)
	{
		const TOptional<FHhAxialCoord> Free = F.Inventory->FindFreeLocation();
		if (!TestTrue(FString::Printf(TEXT("Free cell #%d found"), i), Free.IsSet()))
		{
			return false;
		}
		TestTrue(TEXT("Free cell is in bounds"), F.Inventory->IsLocationInBounds(Free.GetValue()));
		TestFalse(TEXT("Free cell is new"), Used.Contains(Free.GetValue()));
		Used.Add(Free.GetValue());
		F.Add(Free.GetValue());
	}
	TestFalse(TEXT("Full inventory has no free cell"), F.Inventory->FindFreeLocation().IsSet());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryTransferWithinTest, "TypicalShooter.Inventory.Core.TransferWithinInventory", InventoryCoreTest::Flags)
bool FInventoryTransferWithinTest::RunTest(const FString& Parameters)
{
	using namespace InventoryCoreTest;
	FFixture F;
	UInventoryItem* Medkit = F.Inventory->AddItem(NamedManifest(TEXT("Medkit")), CellA);
	UInventoryItem* Ammo = F.Inventory->AddItem(NamedManifest(TEXT("Ammo")), CellB);

	TestTrue(TEXT("Transfer to a free cell of the same inventory"), F.Inventory->TransferItem(Medkit, F.Inventory, CellC));
	TestTrue(TEXT("Same item object, new cell"), F.LocationOf(Medkit) == TOptional<FHhAxialCoord>(CellC));
	TestEqual(TEXT("Reported as a move"), F.NumMoved, 1);

	TestTrue(TEXT("Transfer onto an occupied cell swaps"), F.Inventory->TransferItem(Medkit, F.Inventory, CellB));
	TestTrue(TEXT("Dragged item took the target cell"), F.LocationOf(Medkit) == TOptional<FHhAxialCoord>(CellB));
	TestTrue(TEXT("Occupant took the dragged item's cell"), F.LocationOf(Ammo) == TOptional<FHhAxialCoord>(CellC));
	TestEqual(TEXT("Still two items"), F.Inventory->GetNumItems(), 2);
	TestEqual(TEXT("No adds/removes for a swap"), F.NumRemoved, 0);

	TestFalse(TEXT("Transfer outside the grid fails"), F.Inventory->TransferItem(Medkit, F.Inventory, FHhAxialCoord(5, 0)));
	TestFalse(TEXT("Transfer of an item from another inventory fails"),
		F.Inventory->TransferItem(F.MakeInventory()->AddItem(FInventoryItemManifest(), CellA), F.Inventory, CellD));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryTransferBetweenTest, "TypicalShooter.Inventory.Core.TransferBetweenInventories", InventoryCoreTest::Flags)
bool FInventoryTransferBetweenTest::RunTest(const FString& Parameters)
{
	using namespace InventoryCoreTest;
	FFixture F;
	UInventoryComponent* Player = F.MakeInventory(1);

	UInventoryItem* Medkit = F.Inventory->AddItem(NamedManifest(TEXT("Medkit")), CellA);
	UInventoryItem* Scope = Player->AddItem(NamedManifest(TEXT("Scope")), CellB);

	// To a free cell: the item leaves the source and appears in the target.
	TestTrue(TEXT("Transfer to a free cell of another inventory"), F.Inventory->TransferItem(Medkit, Player, CellC));
	TestEqual(TEXT("Source is empty"), F.Inventory->GetNumItems(), 0);
	TestEqual(TEXT("Target has two items"), Player->GetNumItems(), 2);
	UInventoryItem* Moved = FFixture::ItemAt(Player, CellC);
	TestEqual(TEXT("Target cell holds the transferred item"), NameOf(Moved), FString(TEXT("Medkit")));
	TestTrue(TEXT("Transferred item is owned by the target inventory"), Moved && Moved->GetOuter() == Player);
	TestEqual(TEXT("Source broadcast a removal"), F.NumRemoved, 1);

	// Onto an occupied cell: the two items swap inventories.
	UInventoryItem* Grenade = F.Inventory->AddItem(NamedManifest(TEXT("Grenade")), CellD);
	TestTrue(TEXT("Transfer onto an occupied cell of another inventory swaps"), F.Inventory->TransferItem(Grenade, Player, CellB));
	TestEqual(TEXT("Target cell now holds the dragged item"), NameOf(FFixture::ItemAt(Player, CellB)), FString(TEXT("Grenade")));
	TestEqual(TEXT("Source cell now holds the former occupant"), NameOf(FFixture::ItemAt(F.Inventory, CellD)), FString(TEXT("Scope")));
	TestEqual(TEXT("Source item count unchanged by a swap"), F.Inventory->GetNumItems(), 1);
	TestEqual(TEXT("Target item count unchanged by a swap"), Player->GetNumItems(), 2);

	// Out of the target's bounds: nothing changes.
	UInventoryItem* Scope2 = FFixture::ItemAt(F.Inventory, CellD);
	TestFalse(TEXT("Transfer outside the target grid fails"), F.Inventory->TransferItem(Scope2, Player, FHhAxialCoord(2, 0)));
	TestTrue(TEXT("Item stays in the source"), F.LocationOf(Scope2) == TOptional<FHhAxialCoord>(CellD));

	TestFalse(TEXT("Transfer to a null inventory fails"), F.Inventory->TransferItem(Scope2, nullptr, CellA));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryTransferToFreeTest, "TypicalShooter.Inventory.Core.TransferToFreeLocation", InventoryCoreTest::Flags)
bool FInventoryTransferToFreeTest::RunTest(const FString& Parameters)
{
	using namespace InventoryCoreTest;
	FFixture F;
	UInventoryComponent* Player = F.MakeInventory(0); // a single cell
	UInventoryItem* Medkit = F.Inventory->AddItem(NamedManifest(TEXT("Medkit")), CellB);
	UInventoryItem* Ammo = F.Inventory->AddItem(NamedManifest(TEXT("Ammo")), CellC);

	TestTrue(TEXT("Taking into an empty inventory succeeds"), F.Inventory->TransferItemToFreeLocation(Medkit, Player));
	TestEqual(TEXT("Item landed in the first free cell"), NameOf(FFixture::ItemAt(Player, FHhAxialCoord(0, 0))), FString(TEXT("Medkit")));
	TestEqual(TEXT("Item left the source"), F.Inventory->GetNumItems(), 1);

	TestFalse(TEXT("Taking into a full inventory fails"), F.Inventory->TransferItemToFreeLocation(Ammo, Player));
	TestTrue(TEXT("Item stays in the source"), F.LocationOf(Ammo) == TOptional<FHhAxialCoord>(CellC));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryOwnerFromArchetypeTest, "TypicalShooter.Inventory.Core.OwnerSurvivesArchetypeCopy", InventoryCoreTest::Flags)
bool FInventoryOwnerFromArchetypeTest::RunTest(const FString& Parameters)
{
	// Regression: the inventory array's owner pointer used to be copied from the class default object's component
	// when a Blueprint actor was spawned, so items were outered to (and events broadcast on) the CDO's component.
	using namespace InventoryCoreTest;
	FFixture F;

	UClass* ContainerClass = LoadClass<AItemsContainer>(nullptr, TEXT("/Game/Inventory/Bllueprints/BP_ItemsContainer.BP_ItemsContainer_C"));
	if (!TestNotNull(TEXT("BP_ItemsContainer class loads"), ContainerClass))
	{
		return false;
	}

	for (UClass* Class : { AItemsContainer::StaticClass(), ContainerClass })
	{
		AItemsContainer* Container = F.World->SpawnActor<AItemsContainer>(Class);
		UInventoryComponent* Inventory = Container->InventoryComponent;
		int32 NumAdded = 0;
		Inventory->GetItemAddedDelegate().AddLambda([&NumAdded](const UInventoryItem*, const FHhAxialCoord&) { ++NumAdded; });

		const TOptional<FHhAxialCoord> Free = Inventory->FindFreeLocation();
		UInventoryItem* Item = Free.IsSet() ? Inventory->AddItem(FInventoryItemManifest(), Free.GetValue()) : nullptr;
		if (TestNotNull(*FString::Printf(TEXT("%s: item added"), *Class->GetName()), Item))
		{
			TestTrue(*FString::Printf(TEXT("%s: item is outered to the instance's component"), *Class->GetName()), Item->GetOuter() == Inventory);
		}
		TestEqual(*FString::Printf(TEXT("%s: added event fired on the instance's component"), *Class->GetName()), NumAdded, 1);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryTransferAuthorityTest, "TypicalShooter.Inventory.Core.TransferNeedsAuthority", InventoryCoreTest::Flags)
bool FInventoryTransferAuthorityTest::RunTest(const FString& Parameters)
{
	using namespace InventoryCoreTest;
	FFixture F;
	UInventoryComponent* Player = F.MakeInventory();
	UInventoryItem* Item = F.Add(CellA);

	F.Actor->SetRole(ROLE_SimulatedProxy);
	TestFalse(TEXT("Client cannot transfer directly"), F.Inventory->TransferItem(Item, Player, CellB));
	TestEqual(TEXT("Nothing moved"), Player->GetNumItems(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryRequestTransferTest, "TypicalShooter.Inventory.Core.RequestTransferWithoutController", InventoryCoreTest::Flags)
bool FInventoryRequestTransferTest::RunTest(const FString& Parameters)
{
	// Model-level request used by the UI. With authority and no player controller (standalone tools, tests)
	// it applies directly; with a controller it goes through the controller's server RPC.
	using namespace InventoryCoreTest;
	FFixture F;
	UInventoryComponent* Player = F.MakeInventory();
	UInventoryItem* Item = F.Add(CellA);

	IItemsContainer_Model* Source = F.Inventory;
	Source->RequestTransferItem(Item, TScriptInterface<IItemsContainer_Model>(Player), CellB);
	TestEqual(TEXT("Item left the source"), F.Inventory->GetNumItems(), 0);
	TestNotNull(TEXT("Item arrived at the target cell"), FFixture::ItemAt(Player, CellB));
	TestEqual(TEXT("Model reports its grid radius"), Source->GetGridRadius(), F.Inventory->GetGridRadius());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
