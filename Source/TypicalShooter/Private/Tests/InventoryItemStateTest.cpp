// Per-instance item state (rotation) and keeping it when items move, swap or change inventories (power grid R3a).

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/InventoryTestFixture.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryItemRotationTest, "TypicalShooter.Inventory.Core.ItemRotation", InventoryCoreTest::Flags)
bool FInventoryItemRotationTest::RunTest(const FString& Parameters)
{
	InventoryCoreTest::FFixture F;
	UInventoryItem* Item = F.Add(InventoryCoreTest::CellA);
	if (!TestNotNull(TEXT("Item added"), Item))
	{
		return false;
	}
	TestEqual(TEXT("New item is not rotated"), Item->GetRotation(), 0);
	Item->SetRotation(4);
	TestEqual(TEXT("Rotation is stored"), Item->GetRotation(), 4);
	Item->SetRotation(7);
	TestEqual(TEXT("Rotation wraps after a full turn"), Item->GetRotation(), 1);
	Item->SetRotation(-1);
	TestEqual(TEXT("Negative rotation wraps counter-clockwise"), Item->GetRotation(), 5);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryTransferKeepsStateTest, "TypicalShooter.Inventory.Core.TransferKeepsItemState", InventoryCoreTest::Flags)
bool FInventoryTransferKeepsStateTest::RunTest(const FString& Parameters)
{
	using namespace InventoryCoreTest;
	FFixture F;
	UInventoryComponent* Container = F.MakeInventory();
	UInventoryItem* Medkit = F.Inventory->AddItem(NamedManifest(TEXT("Medkit")), CellA);
	UInventoryItem* Scope = Container->AddItem(NamedManifest(TEXT("Scope")), CellB);
	if (!TestNotNull(TEXT("Medkit added"), Medkit) || !TestNotNull(TEXT("Scope added"), Scope))
	{
		return false;
	}
	Medkit->SetRotation(2);
	Scope->SetRotation(5);

	TestTrue(TEXT("Move within the inventory"), F.Inventory->TransferItem(Medkit, F.Inventory, CellC));
	TestEqual(TEXT("Moved item keeps its rotation"), Medkit->GetRotation(), 2);

	TestTrue(TEXT("Transfer into an empty cell of another inventory"), F.Inventory->TransferItem(Medkit, Container, CellA));
	const UInventoryItem* MovedMedkit = FFixture::ItemAt(Container, CellA);
	if (TestEqual(TEXT("Medkit is in the container"), NameOf(MovedMedkit), FString(TEXT("Medkit"))))
	{
		TestEqual(TEXT("Transferred item keeps its rotation"), MovedMedkit->GetRotation(), 2);
	}

	UInventoryItem* Grenade = F.Inventory->AddItem(NamedManifest(TEXT("Grenade")), CellD);
	Grenade->SetRotation(3);
	TestTrue(TEXT("Swap with an item of another inventory"), F.Inventory->TransferItem(Grenade, Container, CellB));
	const UInventoryItem* SwappedScope = FFixture::ItemAt(F.Inventory, CellD);
	const UInventoryItem* SwappedGrenade = FFixture::ItemAt(Container, CellB);
	if (TestEqual(TEXT("Scope came back to the player"), NameOf(SwappedScope), FString(TEXT("Scope")))
		&& TestEqual(TEXT("Grenade went to the container"), NameOf(SwappedGrenade), FString(TEXT("Grenade"))))
	{
		TestEqual(TEXT("Swapped-out occupant keeps its rotation"), SwappedScope->GetRotation(), 5);
		TestEqual(TEXT("Swapped-in item keeps its rotation"), SwappedGrenade->GetRotation(), 3);
	}
	return true;
}

#endif
