// Fast tests for the inventory core: UInventoryComponent / FInventoryArray, manifests and items.
// Each test spins up a bare game world with one actor owning a UInventoryComponent (no PIE).

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/Engine.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "InventoryComponent.h"
#include "InventoryItem.h"
#include "InventoryItemManifest.h"
#include "ItemFragment.h"
#include "Tests/InventoryTestFixture.h"


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryAddTest, "TypicalShooter.Inventory.Core.Add", InventoryCoreTest::Flags)
bool FInventoryAddTest::RunTest(const FString& Parameters)
{
	InventoryCoreTest::FFixture F;

	UInventoryItem* Item = F.Add(InventoryCoreTest::CellB);
	if (!TestNotNull(TEXT("AddItem returns the created item"), Item))
	{
		return false;
	}
	TestEqual(TEXT("One item in the inventory"), F.Inventory->GetNumItems(), 1);
	TestTrue(TEXT("Item is placed at the requested cell"), F.LocationOf(Item) == TOptional<FHhAxialCoord>(InventoryCoreTest::CellB));
	TestEqual(TEXT("Added broadcast once"), F.NumAdded, 1);
	TestTrue(TEXT("Added broadcast carries the item"), F.LastItem == Item);
	TestTrue(TEXT("Added broadcast carries the cell"), F.LastLocation == InventoryCoreTest::CellB);
	TestTrue(TEXT("Item is owned by the inventory component"), Item->GetOuter() == F.Inventory);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryAddOccupiedTest, "TypicalShooter.Inventory.Core.AddToOccupiedCellFails", InventoryCoreTest::Flags)
bool FInventoryAddOccupiedTest::RunTest(const FString& Parameters)
{
	InventoryCoreTest::FFixture F;

	UInventoryItem* First = F.Add(InventoryCoreTest::CellA);
	UInventoryItem* Second = F.Add(InventoryCoreTest::CellA);

	TestNotNull(TEXT("First item added"), First);
	TestNull(TEXT("Second item into the same cell is rejected"), Second);
	TestEqual(TEXT("Still one item"), F.Inventory->GetNumItems(), 1);
	TestEqual(TEXT("Added broadcast only for the first item"), F.NumAdded, 1);
	TestTrue(TEXT("First item kept its cell"), F.LocationOf(First) == TOptional<FHhAxialCoord>(InventoryCoreTest::CellA));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryAddSameItemTwiceTest, "TypicalShooter.Inventory.Core.AddSameItemTwiceFails", InventoryCoreTest::Flags)
bool FInventoryAddSameItemTwiceTest::RunTest(const FString& Parameters)
{
	InventoryCoreTest::FFixture F;

	UInventoryItem* Item = F.Add(InventoryCoreTest::CellA);
	TestFalse(TEXT("Re-adding an item that is already placed fails"), F.Inventory->AddItem(Item, InventoryCoreTest::CellB));
	TestFalse(TEXT("Adding a null item fails"), F.Inventory->AddItem(static_cast<UInventoryItem*>(nullptr), InventoryCoreTest::CellB));
	TestEqual(TEXT("Still one item"), F.Inventory->GetNumItems(), 1);
	TestTrue(TEXT("Item stays where it was"), F.LocationOf(Item) == TOptional<FHhAxialCoord>(InventoryCoreTest::CellA));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryNoAuthorityTest, "TypicalShooter.Inventory.Core.MutationsNeedAuthority", InventoryCoreTest::Flags)
bool FInventoryNoAuthorityTest::RunTest(const FString& Parameters)
{
	InventoryCoreTest::FFixture F;
	UInventoryItem* Item = F.Add(InventoryCoreTest::CellA);
	F.NumAdded = 0;

	F.Actor->SetRole(ROLE_SimulatedProxy);

	TestNull(TEXT("Client cannot add"), F.Add(InventoryCoreTest::CellB));
	TestFalse(TEXT("Client cannot move"), F.Inventory->MoveItem(Item, InventoryCoreTest::CellC));
	F.Inventory->RemoveItem(Item);
	TestEqual(TEXT("Client cannot remove"), F.Inventory->GetNumItems(), 1);
	TestTrue(TEXT("Item unchanged"), F.LocationOf(Item) == TOptional<FHhAxialCoord>(InventoryCoreTest::CellA));
	TestEqual(TEXT("No broadcasts on a client"), F.NumAdded + F.NumMoved + F.NumRemoved, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryMoveTest, "TypicalShooter.Inventory.Core.Move", InventoryCoreTest::Flags)
bool FInventoryMoveTest::RunTest(const FString& Parameters)
{
	InventoryCoreTest::FFixture F;
	UInventoryItem* Item = F.Add(InventoryCoreTest::CellA);
	UInventoryItem* Other = F.Add(InventoryCoreTest::CellC);

	TestTrue(TEXT("Move to a free cell succeeds"), F.Inventory->MoveItem(Item, InventoryCoreTest::CellB));
	TestTrue(TEXT("Item is at the new cell"), F.LocationOf(Item) == TOptional<FHhAxialCoord>(InventoryCoreTest::CellB));
	TestEqual(TEXT("Moved broadcast once"), F.NumMoved, 1);
	TestTrue(TEXT("Moved broadcast carries new cell"), F.LastLocation == InventoryCoreTest::CellB);
	TestTrue(TEXT("Moved broadcast carries previous cell"), F.LastPrevLocation == InventoryCoreTest::CellA);

	TestTrue(TEXT("Old cell is free again"), F.Add(InventoryCoreTest::CellA) != nullptr);

	TestFalse(TEXT("Move onto an occupied cell fails"), F.Inventory->MoveItem(Item, InventoryCoreTest::CellC));
	TestTrue(TEXT("Item did not move"), F.LocationOf(Item) == TOptional<FHhAxialCoord>(InventoryCoreTest::CellB));
	TestTrue(TEXT("Occupant did not move"), F.LocationOf(Other) == TOptional<FHhAxialCoord>(InventoryCoreTest::CellC));

	TestTrue(TEXT("Move to the same cell is a no-op success"), F.Inventory->MoveItem(Item, InventoryCoreTest::CellB));
	TestEqual(TEXT("No extra Moved broadcasts"), F.NumMoved, 1);

	TestFalse(TEXT("Moving an unknown item fails"), F.Inventory->MoveItem(static_cast<UInventoryItem*>(nullptr), InventoryCoreTest::CellD));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryRemoveTest, "TypicalShooter.Inventory.Core.Remove", InventoryCoreTest::Flags)
bool FInventoryRemoveTest::RunTest(const FString& Parameters)
{
	InventoryCoreTest::FFixture F;
	UInventoryItem* ItemA = F.Add(InventoryCoreTest::CellA);
	UInventoryItem* ItemB = F.Add(InventoryCoreTest::CellB);
	UInventoryItem* ItemC = F.Add(InventoryCoreTest::CellC);

	// Removing the first entry swaps the last one into its slot: the rest must keep their cells.
	F.Inventory->RemoveItem(ItemA);
	TestEqual(TEXT("Two items left"), F.Inventory->GetNumItems(), 2);
	TestFalse(TEXT("Removed item is gone"), F.LocationOf(ItemA).IsSet());
	TestTrue(TEXT("B keeps its cell"), F.LocationOf(ItemB) == TOptional<FHhAxialCoord>(InventoryCoreTest::CellB));
	TestTrue(TEXT("C keeps its cell"), F.LocationOf(ItemC) == TOptional<FHhAxialCoord>(InventoryCoreTest::CellC));
	TestEqual(TEXT("Removed broadcast once"), F.NumRemoved, 1);
	TestTrue(TEXT("Removed broadcast carries the item"), F.LastItem == ItemA);

	// Remove by cell, including the last array element.
	F.Inventory->RemoveItem(InventoryCoreTest::CellC);
	TestFalse(TEXT("Item at C removed by location"), F.LocationOf(ItemC).IsSet());
	TestTrue(TEXT("B still placed"), F.LocationOf(ItemB) == TOptional<FHhAxialCoord>(InventoryCoreTest::CellB));

	F.Inventory->RemoveItem(InventoryCoreTest::CellB);
	TestEqual(TEXT("Inventory empty"), F.Inventory->GetNumItems(), 0);
	TestEqual(TEXT("Three Removed broadcasts"), F.NumRemoved, 3);

	F.Inventory->RemoveItem(InventoryCoreTest::CellD);
	F.Inventory->RemoveItem(ItemA);
	TestEqual(TEXT("Removing missing cells/items is a no-op"), F.NumRemoved, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryManifestFragmentsTest, "TypicalShooter.Inventory.Core.ManifestFragments", InventoryCoreTest::Flags)
bool FInventoryManifestFragmentsTest::RunTest(const FString& Parameters)
{
	FInventoryItemManifest Manifest;
	TestNull(TEXT("No fragment in an empty manifest"), Manifest.GetFragmentOfType<FInfo_ItemFragment>());

	FGrid_ItemFragment Grid;
	Grid.Size = 3;
	Manifest.Fragments.Add(TInstancedStruct<FItemFragment>::Make<FGrid_ItemFragment>(Grid));
	FInfo_ItemFragment Info;
	Info.DisplayName = FText::FromString(TEXT("Medkit"));
	Manifest.Fragments.Add(TInstancedStruct<FItemFragment>::Make<FInfo_ItemFragment>(Info));

	const FGrid_ItemFragment* FoundGrid = Manifest.GetFragmentOfType<FGrid_ItemFragment>();
	const FInfo_ItemFragment* FoundInfo = Manifest.GetFragmentOfType<FInfo_ItemFragment>();
	if (TestNotNull(TEXT("Grid fragment found by type"), FoundGrid))
	{
		TestEqual(TEXT("Grid fragment data"), FoundGrid->Size, 3);
		TestTrue(TEXT("Returned pointer refers to the manifest's own storage"),
			FoundGrid == Manifest.Fragments[0].GetPtr<FGrid_ItemFragment>());
	}
	if (TestNotNull(TEXT("Info fragment found by type"), FoundInfo))
	{
		TestEqual(TEXT("Info fragment data"), FoundInfo->DisplayName.ToString(), FString(TEXT("Medkit")));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryItemModelTest, "TypicalShooter.Inventory.Core.ItemModel", InventoryCoreTest::Flags)
bool FInventoryItemModelTest::RunTest(const FString& Parameters)
{
	InventoryCoreTest::FFixture F;

	// Without an Info fragment the item falls back to its object name and an empty brush.
	UInventoryItem* Plain = F.Add(InventoryCoreTest::CellA);
	TestEqual(TEXT("Fallback name is the object name"), Plain->GetItemName().ToString(), Plain->GetName());
	TestTrue(TEXT("Fallback description is empty"), Plain->GetItemDescription().IsEmpty());
	TestNull(TEXT("Fallback icon has no resource"), Plain->GetItemIcon().GetResourceObject());
	TestEqual(TEXT("Count is 1 (no stacking yet)"), Plain->GetItemCount(), 1);

	FInventoryItemManifest Manifest;
	FInfo_ItemFragment Info;
	Info.DisplayName = FText::FromString(TEXT("Medkit"));
	Info.Description = FText::FromString(TEXT("Restores health"));
	Info.Icon = UTexture2D::CreateTransient(8, 4);
	Manifest.Fragments.Add(TInstancedStruct<FItemFragment>::Make<FInfo_ItemFragment>(Info));

	UInventoryItem* Described = F.Inventory->AddItem(Manifest, InventoryCoreTest::CellB);
	if (!TestNotNull(TEXT("Item with Info fragment added"), Described))
	{
		return false;
	}
	TestEqual(TEXT("Name from Info fragment"), Described->GetItemName().ToString(), FString(TEXT("Medkit")));
	TestEqual(TEXT("Description from Info fragment"), Described->GetItemDescription().ToString(), FString(TEXT("Restores health")));
	const FSlateBrush Icon = Described->GetItemIcon();
	TestTrue(TEXT("Icon brush uses the Info texture"), Icon.GetResourceObject() == Info.Icon);
	TestTrue(TEXT("Icon brush size matches the texture"), FVector2D(Icon.ImageSize).Equals(FVector2D(8.0, 4.0)));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
