// The power grid drawn over the inventory grid (power grid R16): face segment geometry, cell labels, and the container
// presenter feeding the grid view with the power grid display on every power grid change.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "HhGridFunctions.h"
#include "PowerGrid/PowerGridComponent.h"
#include "PowerGrid/PowerGridFragment.h"
#include "Tests/InventoryTestFixture.h"
#include "UObject/StrongObjectPtr.h"
#include "UI/ItemsContainer/ItemsContainer_Presenter.h"
#include "UI/Slate/InventoryGrid.h"
#include "UI/Slate/PowerGridOverlay.h"

namespace PowerGridOverlayTest
{
	FInventoryItemManifest Power(const EPowerItemType Type, const int32 Faces = HexFaces::AllFaces)
	{
		FPowerGrid_ItemFragment Fragment;
		Fragment.ItemType = Type;
		Fragment.OpenFaces = Faces;
		Fragment.Receiver.RequiredPower = 2;
		FInventoryItemManifest Manifest;
		Manifest.Fragments.Add(TInstancedStruct<FItemFragment>::Make<FPowerGrid_ItemFragment>(Fragment));
		return Manifest;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerGridOverlayGeometryTest, "TypicalShooter.Inventory.UI.PowerGridOverlayGeometry", InventoryCoreTest::Flags)
bool FPowerGridOverlayGeometryTest::RunTest(const FString& Parameters)
{
	constexpr float HexSize = 40.f;
	const FVector2D Size(400.f, 300.f);
	TestTrue(TEXT("Cell centres: (0,0) in the middle of the widget"), PowerGridOverlay::GetCellCenter(FHhAxialCoord(0, 0), HexSize, Size).Equals(Size * 0.5f));
	TestTrue(TEXT("... others offset like the grid panel"),
		PowerGridOverlay::GetCellCenter(FHhAxialCoord(1, 0), HexSize, Size).Equals(Size * 0.5f + UHhGridFunctions::HexAxialCoordToPoint(FHhAxialCoord(1, 0), HexSize)));

	// Open faces are drawn by the item underlay: geometry tested in TypicalShooter.Loot.UI.Underlay.SpokeGeometry.

	FPowerCellDisplay Cell;
	Cell.Power = 2;
	TestEqual(TEXT("Power label"), PowerGridOverlay::GetPowerLabel(Cell).ToString(), FString(TEXT("2")));
	Cell.Type = EPowerItemType::Receiver;
	Cell.RequiredPower = 3;
	TestEqual(TEXT("Receiver label: fed / required"), PowerGridOverlay::GetPowerLabel(Cell).ToString(), FString(TEXT("2/3")));
	Cell.Type = EPowerItemType::Source;
	Cell.Clip = FIntPoint(1, 4);
	TestEqual(TEXT("Ammo source label: power and clip"), PowerGridOverlay::GetPowerLabel(Cell).ToString(), FString(TEXT("2 [1/4]")));
	TestFalse(TEXT("Slot cells are labelled"), PowerGridOverlay::GetSlotLabel(ReceiverSlots::GetLocation(EReceiverSlot::Air)).IsEmpty());
	TestTrue(TEXT("Other cells are not"), PowerGridOverlay::GetSlotLabel(FHhAxialCoord(0, 0)).IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerGridOnGridTest, "TypicalShooter.Inventory.UI.PowerGridOnGrid", InventoryCoreTest::Flags)
bool FPowerGridOnGridTest::RunTest(const FString& Parameters)
{
	using namespace PowerGridOverlayTest;
	InventoryCoreTest::FFixture F;
	F.Inventory->AddItem(Power(EPowerItemType::Source), FHhAxialCoord(0, 0));
	const TSharedRef<SInventoryGrid> Grid = SNew(SInventoryGrid);
	const TStrongObjectPtr<UItemsContainer_Presenter> Presenter(UItemsContainer_Presenter::CreatePresenter(F.Inventory));
	Presenter->SetView(Grid);
	TestEqual(TEXT("No power grid on the actor: nothing to draw"), Grid->GetPowerGrid().Cells.Num(), 0);

	NewObject<UPowerGridComponent>(F.Actor)->RegisterComponent();
	Presenter->SetModel(F.Inventory);
	TestEqual(TEXT("The presenter finds the inventory's power grid"), Grid->GetPowerGrid().Cells.Num(), 1);

	UInventoryItem* Connector = F.Inventory->AddItem(Power(EPowerItemType::Connector, HexFaces::Bit(EHexFace::Left)), FHhAxialCoord(1, 0));
	TestEqual(TEXT("A power grid change reaches the view"), Grid->GetPowerGrid().Cells.Num(), 2);
	TestEqual(TEXT("... with the link"), Grid->GetPowerGrid().Links.Num(), 1);
	F.Inventory->RotateItem(Connector, 1);
	const FPowerCellDisplay* Rotated = Grid->GetPowerGrid().FindCell(FHhAxialCoord(1, 0));
	TestTrue(TEXT("Rotation updates the drawn faces"), Rotated && Rotated->OpenFaces == HexFaces::Bit(EHexFace::TopLeft));
	TestEqual(TEXT("... and drops the link"), Grid->GetPowerGrid().Links.Num(), 0);

	Presenter->SetView(nullptr);
	return true;
}

#endif
