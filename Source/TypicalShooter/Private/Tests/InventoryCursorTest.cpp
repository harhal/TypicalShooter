// Gamepad / keyboard cursor of the inventory UI (UInventoryUISubsystem): hex navigation, switching between grids,
// selecting and moving items with "confirm". Bare game world, no PIE, no input layer (that part is in UIFlow).

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/LocalPlayer.h"
#include "HhGridFunctions.h"
#include "Tests/InventoryTestFixture.h"
#include "UI/InventoryUISubsystem.h"
#include "UI/Slate/InventoryGrid.h"

namespace InventoryCursorTest
{
	const FVector2D Right(1.f, 0.f);
	const FVector2D Left(-1.f, 0.f);
	const FVector2D Up(0.f, 1.f);
	const FVector2D Down(0.f, -1.f);

	UInventoryUISubsystem* MakeSubsystem()
	{
		// Local player subsystems must live inside a local player; the logic needs nothing else.
		ULocalPlayer* LocalPlayer = NewObject<ULocalPlayer>(GEngine);
		return NewObject<UInventoryUISubsystem>(LocalPlayer);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryCursorMoveTest, "TypicalShooter.Inventory.UI.Cursor.HexNavigation", InventoryCoreTest::Flags)
bool FInventoryCursorMoveTest::RunTest(const FString& Parameters)
{
	using namespace InventoryCoreTest;
	using namespace InventoryCursorTest;
	FFixture F; // radius 2
	UInventoryUISubsystem* UI = MakeSubsystem();

	TestNull(TEXT("No cursor container before the inventory is opened"), UI->GetCursorContainer());
	TestFalse(TEXT("Moving without containers does nothing"), UI->MoveCursor(Right));

	UI->SetNavigableContainers({ F.Inventory });
	TestTrue(TEXT("Cursor starts in the first container"), UI->GetCursorContainer() == F.Inventory);
	TestTrue(TEXT("Cursor starts in the centre"), UI->GetCursorLocation() == FHhAxialCoord(0, 0));

	TestTrue(TEXT("Right"), UI->MoveCursor(Right));
	TestTrue(TEXT("Right goes to (1,0)"), UI->GetCursorLocation() == FHhAxialCoord(1, 0));
	TestTrue(TEXT("Moving activates the cursor"), UI->IsCursorActive());
	UI->MoveCursor(Left);
	TestTrue(TEXT("Left comes back"), UI->GetCursorLocation() == FHhAxialCoord(0, 0));

	// Pointy-top hexes: straight up alternates between the two upper neighbours, staying in one column.
	UI->MoveCursor(Up);
	TestTrue(TEXT("Up goes to an upper neighbour"), UHhGridFunctions::GetHexDistance(UI->GetCursorLocation(), FHhAxialCoord(0, 0)) == 1
		&& UI->GetCursorLocation().CellCoord.Y == -1);
	UI->MoveCursor(Up);
	TestTrue(TEXT("Up twice stays in the starting column: (1,-2)"), UI->GetCursorLocation() == FHhAxialCoord(1, -2));
	UI->MoveCursor(Down);
	UI->MoveCursor(Down);
	TestTrue(TEXT("Down twice returns to the centre"), UI->GetCursorLocation() == FHhAxialCoord(0, 0));

	// Diagonals pick the matching hex direction.
	UI->MoveCursor(FVector2D(1.f, -1.f));
	TestTrue(TEXT("Down-right goes to (0,1)"), UI->GetCursorLocation() == FHhAxialCoord(0, 1));

	// Grid edge: radius 2.
	UI->MoveCursor(Left);
	UI->MoveCursor(Left);
	TestTrue(TEXT("At the left edge"), UI->GetCursorLocation() == FHhAxialCoord(-2, 1));
	TestFalse(TEXT("Cannot leave the grid"), UI->MoveCursor(Left));
	TestTrue(TEXT("Cursor stays on the edge"), UI->GetCursorLocation() == FHhAxialCoord(-2, 1));
	TestFalse(TEXT("A zero direction does nothing"), UI->MoveCursor(FVector2D::ZeroVector));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryCursorSwitchTest, "TypicalShooter.Inventory.UI.Cursor.SwitchContainers", InventoryCoreTest::Flags)
bool FInventoryCursorSwitchTest::RunTest(const FString& Parameters)
{
	using namespace InventoryCoreTest;
	using namespace InventoryCursorTest;
	FFixture F;
	UInventoryComponent* Container = F.MakeInventory(1);
	UInventoryUISubsystem* UI = MakeSubsystem();

	UI->SetNavigableContainers({ F.Inventory, Container });
	UI->MoveCursor(Right);
	UI->SwitchCursorContainer(1);
	TestTrue(TEXT("Switched to the container"), UI->GetCursorContainer() == Container);
	TestTrue(TEXT("New container starts at the centre"), UI->GetCursorLocation() == FHhAxialCoord(0, 0));
	UI->MoveCursor(Down);

	UI->SwitchCursorContainer(1);
	TestTrue(TEXT("Switching wraps around"), UI->GetCursorContainer() == F.Inventory);
	TestTrue(TEXT("Each container remembers its cursor cell"), UI->GetCursorLocation() == FHhAxialCoord(1, 0));
	UI->SwitchCursorContainer(-1);
	TestTrue(TEXT("Switching back"), UI->GetCursorContainer() == Container);
	TestTrue(TEXT("Container cursor cell remembered"), UI->GetCursorLocation().CellCoord.Y == 1);

	UI->SetNavigableContainers({ F.Inventory });
	TestTrue(TEXT("Closing the cursor's container moves the cursor to the remaining one"), UI->GetCursorContainer() == F.Inventory);

	UI->SetNavigableContainers({});
	TestNull(TEXT("No containers, no cursor"), UI->GetCursorContainer());
	TestFalse(TEXT("Cursor deactivated"), UI->IsCursorActive());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryCursorConfirmTest, "TypicalShooter.Inventory.UI.Cursor.ConfirmMovesItems", InventoryCoreTest::Flags)
bool FInventoryCursorConfirmTest::RunTest(const FString& Parameters)
{
	using namespace InventoryCoreTest;
	using namespace InventoryCursorTest;
	FFixture F; // the "player" inventory; the bare world has no player controller, so requests apply directly
	UInventoryComponent* Container = F.MakeInventory(1);
	UInventoryItem* Medkit = F.Inventory->AddItem(NamedManifest(TEXT("Medkit")), CellA);
	UInventoryItem* Ammo = F.Inventory->AddItem(NamedManifest(TEXT("Ammo")), CellB);
	UInventoryUISubsystem* UI = MakeSubsystem();
	UI->SetNavigableContainers({ F.Inventory, Container });

	TestTrue(TEXT("Item under the cursor"), UI->GetItemAtCursor() == Medkit);
	TestTrue(TEXT("Active cursor: focused/displayed item is the one under it"), (UI->SetCursorActive(true), UI->GetDisplayedItem() == Medkit));

	// Select, then confirm on the same item: deselect.
	UI->ConfirmAtCursor();
	TestTrue(TEXT("Confirm on an item selects it"), UI->GetSelectedItem() == Medkit);
	TestFalse(TEXT("Confirm on the selected item deselects it"), (UI->ConfirmAtCursor(), UI->GetSelectedItem() != nullptr));

	// Move within the inventory onto an empty cell.
	UI->ConfirmAtCursor();
	UI->MoveCursor(Down); // (0,1) is empty
	UI->ConfirmAtCursor();
	TestTrue(TEXT("Confirm on an empty cell moves the selected item there"), F.LocationOf(Medkit) == TOptional<FHhAxialCoord>(FHhAxialCoord(0, 1)));
	TestNull(TEXT("Selection cleared after the move"), UI->GetSelectedItem());
	TestTrue(TEXT("The moved item is now under the cursor"), UI->GetItemAtCursor() == Medkit);

	// Swap within the inventory.
	UI->ConfirmAtCursor();
	UI->MoveCursor(Up);   // (1,0) holds Ammo: up from (0,1) prefers the column (0,0)/(1,0)
	if (UI->GetItemAtCursor() != Ammo)
	{
		UI->MoveCursor(Right);
	}
	TestTrue(TEXT("Cursor on Ammo"), UI->GetItemAtCursor() == Ammo);
	UI->ConfirmAtCursor();
	TestTrue(TEXT("Confirm on another item swaps: Medkit took Ammo's cell"), F.LocationOf(Medkit) == TOptional<FHhAxialCoord>(CellB));
	TestTrue(TEXT("Ammo took Medkit's cell"), F.LocationOf(Ammo) == TOptional<FHhAxialCoord>(FHhAxialCoord(0, 1)));

	// Between inventories.
	UI->ConfirmAtCursor(); // select Medkit (now under the cursor at (1,0))
	TestTrue(TEXT("Medkit selected again"), UI->GetSelectedItem() == Medkit);
	UI->SwitchCursorContainer(1);
	UI->ConfirmAtCursor();
	TestEqual(TEXT("Moved to the other container"), NameOf(FFixture::ItemAt(Container, FHhAxialCoord(0, 0))), FString(TEXT("Medkit")));
	TestEqual(TEXT("Player inventory has one item left"), F.Inventory->GetNumItems(), 1);
	TestNull(TEXT("Selection cleared"), UI->GetSelectedItem());

	// Cancel.
	TestFalse(TEXT("Cancel with nothing selected reports false"), UI->CancelSelection());
	UI->ConfirmAtCursor();
	TestTrue(TEXT("Cancel with a selection reports true"), UI->CancelSelection());
	TestNull(TEXT("Cancel clears the selection"), UI->GetSelectedItem());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryCursorHighlightTest, "TypicalShooter.Inventory.UI.Cursor.GridHighlight", InventoryCoreTest::Flags)
bool FInventoryCursorHighlightTest::RunTest(const FString& Parameters)
{
	using namespace InventoryCoreTest;
	const TSharedRef<SInventoryGrid> Grid = SNew(SInventoryGrid).GridRadius(2);

	Grid->SetCursorCell(CellB);
	TestEqual(TEXT("Cursor cell is highlighted"), Grid->GetCellHighlight(CellB), EInventoryCellHighlight::Cursor);
	Grid->SetHoveredCell(CellB);
	TestEqual(TEXT("Cursor wins over mouse hover"), Grid->GetCellHighlight(CellB), EInventoryCellHighlight::Cursor);
	Grid->SetCursorCell({});
	TestEqual(TEXT("Hidden cursor: hover shows again"), Grid->GetCellHighlight(CellB), EInventoryCellHighlight::Hovered);
	Grid->SetCursorCell(FHhAxialCoord(5, 0));
	TestEqual(TEXT("Cursor outside the grid is ignored"), Grid->GetCellHighlight(FHhAxialCoord(5, 0)), EInventoryCellHighlight::None);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
