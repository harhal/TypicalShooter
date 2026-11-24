// Inventory UI without PIE: container presenter + SInventoryGrid view (cells, drag & drop, hover / selection),
// the selection subsystem and the item info view.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/LocalPlayer.h"
#include "Tests/InventoryTestFixture.h"
#include "UObject/StrongObjectPtr.h"
#include "UI/InventoryItem/InventoryItem_Presenter.h"
#include "UI/InventoryUISubsystem.h"
#include "UI/ItemsContainer/ItemsContainer_Presenter.h"
#include "UI/Slate/InventoryGrid.h"
#include "UI/Slate/InventoryItemInfo.h"

namespace InventoryGridUITest
{
	/** Presenter + Slate grid bound to one inventory. */
	struct FBoundGrid
	{
		TStrongObjectPtr<UItemsContainer_Presenter> Presenter;
		TSharedRef<SInventoryGrid> Grid = SNew(SInventoryGrid);

		explicit FBoundGrid(UInventoryComponent* Inventory)
			: Presenter(UItemsContainer_Presenter::CreatePresenter(Inventory))
		{
			Presenter->SetView(Grid);
		}

		~FBoundGrid()
		{
			Presenter->SetView(nullptr);
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryGridCellsTest, "TypicalShooter.Inventory.UI.GridShowsWholeRadius", InventoryCoreTest::Flags)
bool FInventoryGridCellsTest::RunTest(const FString& Parameters)
{
	using namespace InventoryCoreTest;
	FFixture F;
	F.Inventory->SetGridRadius(1);
	F.Add(CellA);
	F.Add(CellB);

	InventoryGridUITest::FBoundGrid Bound(F.Inventory);
	TestEqual(TEXT("View got the model's radius"), Bound.Grid->GetGridRadius(), 1);
	TestEqual(TEXT("One background cell per grid cell"), Bound.Grid->GetNumCells(), 7);
	TestEqual(TEXT("One view per item"), Bound.Grid->GetNumItemViews(), 2);
	TestTrue(TEXT("Occupied cell has a slot"), Bound.Grid->GetSlotAt(CellA).IsValid());
	TestFalse(TEXT("Empty cell has no slot"), Bound.Grid->GetSlotAt(CellC).IsValid());

	F.Inventory->SetGridRadius(2);
	Bound.Presenter->SetView(Bound.Grid);
	TestEqual(TEXT("Rebuilding picks up a new radius"), Bound.Grid->GetNumCells(), 19);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryGridCoordTest, "TypicalShooter.Inventory.UI.CellUnderCursor", InventoryCoreTest::Flags)
bool FInventoryGridCoordTest::RunTest(const FString& Parameters)
{
	const TSharedRef<SInventoryGrid> Grid = SNew(SInventoryGrid).HexSize(20.f);
	const FVector2D Size(400.f, 300.f);
	const FVector2D Centre = Size * 0.5f;
	TestTrue(TEXT("Widget centre is cell (0,0)"), Grid->GetCoordAtLocalPosition(Centre, Size) == FHhAxialCoord(0, 0));
	TestTrue(TEXT("One hex width right is cell (1,0)"),
		Grid->GetCoordAtLocalPosition(Centre + FVector2D(FMath::Sqrt(3.f) * 20.f, 0.f), Size) == FHhAxialCoord(1, 0));
	TestTrue(TEXT("Down-right neighbour is cell (0,1)"),
		Grid->GetCoordAtLocalPosition(Centre + FVector2D(FMath::Sqrt(3.f) / 2.f * 20.f, 30.f), Size) == FHhAxialCoord(0, 1));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryGridDragWithinTest, "TypicalShooter.Inventory.UI.DragDropWithinGrid", InventoryCoreTest::Flags)
bool FInventoryGridDragWithinTest::RunTest(const FString& Parameters)
{
	using namespace InventoryCoreTest;
	FFixture F;
	UInventoryItem* Medkit = F.Inventory->AddItem(NamedManifest(TEXT("Medkit")), CellA);
	UInventoryItem* Ammo = F.Inventory->AddItem(NamedManifest(TEXT("Ammo")), CellB);
	InventoryGridUITest::FBoundGrid Bound(F.Inventory);

	TestFalse(TEXT("Nothing to drag from an empty cell"), Bound.Grid->MakeDragPayload(CellC).IsSet());

	TOptional<FInventoryItemDragPayload> Payload = Bound.Grid->MakeDragPayload(CellA);
	if (!TestTrue(TEXT("Dragging an item makes a payload"), Payload.IsSet()))
	{
		return false;
	}
	TestTrue(TEXT("Drop on a free cell is accepted"), Bound.Grid->DropPayload(Payload.GetValue(), CellC));
	TestTrue(TEXT("Model moved the item"), F.LocationOf(Medkit) == TOptional<FHhAxialCoord>(CellC));
	TestTrue(TEXT("View followed the model"), Bound.Grid->GetSlotAt(CellC).IsValid() && !Bound.Grid->GetSlotAt(CellA).IsValid());

	Payload = Bound.Grid->MakeDragPayload(CellC);
	TestTrue(TEXT("Drop on an occupied cell is accepted"), Bound.Grid->DropPayload(Payload.GetValue(), CellB));
	TestTrue(TEXT("Dragged item took the cell"), F.LocationOf(Medkit) == TOptional<FHhAxialCoord>(CellB));
	TestTrue(TEXT("Occupant was swapped"), F.LocationOf(Ammo) == TOptional<FHhAxialCoord>(CellC));
	TestEqual(TEXT("Views still match items"), Bound.Grid->GetNumItemViews(), 2);

	Payload = Bound.Grid->MakeDragPayload(CellB);
	TestFalse(TEXT("Drop outside the grid is rejected"), Bound.Grid->DropPayload(Payload.GetValue(), FHhAxialCoord(7, 0)));
	TestTrue(TEXT("Item did not move"), F.LocationOf(Medkit) == TOptional<FHhAxialCoord>(CellB));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryGridDragBetweenTest, "TypicalShooter.Inventory.UI.DragDropBetweenGrids", InventoryCoreTest::Flags)
bool FInventoryGridDragBetweenTest::RunTest(const FString& Parameters)
{
	using namespace InventoryCoreTest;
	FFixture F;
	UInventoryComponent* Player = F.MakeInventory();
	F.Inventory->AddItem(NamedManifest(TEXT("Medkit")), CellA);

	InventoryGridUITest::FBoundGrid ContainerGrid(F.Inventory);
	InventoryGridUITest::FBoundGrid PlayerGrid(Player);

	const TOptional<FInventoryItemDragPayload> Payload = ContainerGrid.Grid->MakeDragPayload(CellA);
	if (!TestTrue(TEXT("Container item can be dragged"), Payload.IsSet()))
	{
		return false;
	}
	TestTrue(TEXT("Drop on the player grid is accepted"), PlayerGrid.Grid->DropPayload(Payload.GetValue(), CellD));
	TestEqual(TEXT("Container is empty"), F.Inventory->GetNumItems(), 0);
	TestEqual(TEXT("Player got the item"), NameOf(FFixture::ItemAt(Player, CellD)), FString(TEXT("Medkit")));
	TestEqual(TEXT("Container grid shows nothing"), ContainerGrid.Grid->GetNumItemViews(), 0);
	TestEqual(TEXT("Player grid shows the item"), PlayerGrid.Grid->GetNumItemViews(), 1);
	TestTrue(TEXT("Player grid shows it at the drop cell"), PlayerGrid.Grid->GetSlotAt(CellD).IsValid());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryGridHoverTest, "TypicalShooter.Inventory.UI.HoverAndClick", InventoryCoreTest::Flags)
bool FInventoryGridHoverTest::RunTest(const FString& Parameters)
{
	using namespace InventoryCoreTest;
	FFixture F;
	UInventoryItem* Medkit = F.Inventory->AddItem(NamedManifest(TEXT("Medkit")), CellA);
	InventoryGridUITest::FBoundGrid Bound(F.Inventory);

	const UInventoryItem* Hovered = nullptr;
	int32 NumHoverEvents = 0;
	const UInventoryItem* Clicked = nullptr;
	Bound.Presenter->OnItemHovered.AddLambda([&](const UInventoryItem* Item) { Hovered = Item; ++NumHoverEvents; });
	Bound.Presenter->OnItemClicked.AddLambda([&](const UInventoryItem* Item) { Clicked = Item; });

	Bound.Grid->SetHoveredCell(CellA);
	TestTrue(TEXT("Hovering an item reports it"), Hovered == Medkit);
	TestEqual(TEXT("Hovered cell is highlighted"), Bound.Grid->GetCellHighlight(CellA), EInventoryCellHighlight::Hovered);
	TestEqual(TEXT("Other cells are not"), Bound.Grid->GetCellHighlight(CellB), EInventoryCellHighlight::None);

	Bound.Grid->SetHoveredCell(CellA);
	TestEqual(TEXT("Hovering the same cell again does not re-report"), NumHoverEvents, 1);

	Bound.Grid->SetHoveredCell(CellB);
	TestNull(TEXT("Hovering an empty cell reports no item"), Hovered);
	Bound.Grid->SetHoveredCell({});
	TestEqual(TEXT("Leaving the grid clears the hover highlight"), Bound.Grid->GetCellHighlight(CellB), EInventoryCellHighlight::None);

	Bound.Grid->ClickCell(CellA);
	TestTrue(TEXT("Clicking an item reports it"), Clicked == Medkit);

	Bound.Grid->SetSelectedSlot(Bound.Grid->GetSlotAt(CellA));
	TestEqual(TEXT("Selected item's cell is highlighted"), Bound.Grid->GetCellHighlight(CellA), EInventoryCellHighlight::Selected);
	TestTrue(TEXT("Presenter maps the slot back to the item"), Bound.Presenter->GetItemInSlot(Bound.Grid->GetSlotAt(CellA)) == Medkit);
	TestTrue(TEXT("Presenter maps the item to its slot"), Bound.Presenter->GetSlotOfItem(Medkit) == Bound.Grid->GetSlotAt(CellA));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventorySelectionTest, "TypicalShooter.Inventory.UI.SelectionSubsystem", InventoryCoreTest::Flags)
bool FInventorySelectionTest::RunTest(const FString& Parameters)
{
	using namespace InventoryCoreTest;
	FFixture F;
	UInventoryItem* Medkit = F.Add(CellA);
	UInventoryItem* Ammo = F.Add(CellB);

	// Local player subsystems must live inside a local player; the subsystem's logic needs nothing else.
	ULocalPlayer* LocalPlayer = NewObject<ULocalPlayer>(GEngine);
	UInventoryUISubsystem* Selection = NewObject<UInventoryUISubsystem>(LocalPlayer);
	int32 NumChanges = 0;
	Selection->OnSelectionChanged.AddLambda([&NumChanges]() { ++NumChanges; });

	TestNull(TEXT("Nothing displayed at start"), Selection->GetDisplayedItem());

	Selection->SetHoveredItem(Medkit);
	TestTrue(TEXT("Hovered item is displayed"), Selection->GetDisplayedItem() == Medkit);

	Selection->SelectItem(Ammo);
	TestTrue(TEXT("Hover wins over selection"), Selection->GetDisplayedItem() == Medkit);
	TestTrue(TEXT("Selection stored"), Selection->GetSelectedItem() == Ammo);

	Selection->SetHoveredItem(nullptr);
	TestTrue(TEXT("Without hover the selected item is displayed"), Selection->GetDisplayedItem() == Ammo);

	Selection->SelectItem(Ammo);
	TestNull(TEXT("Clicking the selected item again deselects it"), Selection->GetSelectedItem());

	Selection->SelectItem(Medkit);
	Selection->SetHoveredItem(Medkit);
	Selection->ForgetItem(Medkit);
	TestNull(TEXT("A removed item is no longer selected"), Selection->GetSelectedItem());
	TestNull(TEXT("A removed item is no longer hovered"), Selection->GetHoveredItem());

	TestTrue(TEXT("Every change was broadcast"), NumChanges >= 6);
	const int32 Before = NumChanges;
	Selection->SetHoveredItem(nullptr);
	TestEqual(TEXT("No broadcast when nothing changes"), NumChanges, Before);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryItemInfoViewTest, "TypicalShooter.Inventory.UI.ItemInfoView", InventoryCoreTest::Flags)
bool FInventoryItemInfoViewTest::RunTest(const FString& Parameters)
{
	using namespace InventoryCoreTest;
	FFixture F;
	FInventoryItemManifest Manifest = NamedManifest(TEXT("Medkit"));
	Manifest.Fragments[0].GetMutable<FInfo_ItemFragment>().Description = FText::FromString(TEXT("Restores health"));
	UInventoryItem* Medkit = F.Inventory->AddItem(Manifest, CellA);

	const TSharedRef<SInventoryItemInfo> Info = SNew(SInventoryItemInfo);
	TStrongObjectPtr<UInventoryItem_Presenter> Presenter(NewObject<UInventoryItem_Presenter>());
	Presenter->SetView(Info);

	TestFalse(TEXT("Empty info shows nothing"), Info->HasItem());

	Presenter->SetModel(Medkit);
	TestTrue(TEXT("Info shows an item"), Info->HasItem());
	TestEqual(TEXT("Info shows the name"), Info->GetDisplayedName().ToString(), FString(TEXT("Medkit")));
	TestEqual(TEXT("Info shows the description"), Info->GetDisplayedDescription().ToString(), FString(TEXT("Restores health")));

	Presenter->SetModel(nullptr);
	TestFalse(TEXT("Clearing the model empties the info"), Info->HasItem());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
