// Tests for the procedural item underlay with outputs (LootGeneration R11): spoke geometry and the grid pushing the
// power grid display's faces into the item views.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "HhGridFunctions.h"
#include "Tests/InventoryTestFixture.h"
#include "UI/ItemsContainer/ItemsContainer_Presenter.h"
#include "UI/Slate/InventoryGrid.h"
#include "UI/Slate/InventoryGridItem.h"
#include "UI/Slate/PowerGridOverlay.h"
#include "UObject/StrongObjectPtr.h"

namespace LootUnderlayTest
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
	const FHhAxialCoord CellA(0, 0);
	const FHhAxialCoord CellB(1, 0);
	const FHhAxialCoord CellC(0, 1);

	FPowerGridDisplay DisplayWith(const FHhAxialCoord& Location, const int32 Faces, const bool bPowered)
	{
		FPowerGridDisplay Display;
		FPowerCellDisplay& Cell = Display.Cells.AddDefaulted_GetRef();
		Cell.Location = Location;
		Cell.OpenFaces = Faces;
		Cell.bPowered = bPowered;
		return Display;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootUnderlaySpokeTest, "TypicalShooter.Loot.UI.Underlay.SpokeGeometry", LootUnderlayTest::Flags)
bool FLootUnderlaySpokeTest::RunTest(const FString& Parameters)
{
	constexpr float HexSize = 40.f;
	const FVector2D Center(100.f, 50.f);
	const float Apothem = HexSize * FMath::Sqrt(3.f) * 0.5f;
	for (int32 Index = 0; Index < HexFaces::Num; ++Index)
	{
		const EHexFace Face = HexFaces::FromIndex(Index);
		const TPair<FVector2D, FVector2D> Spoke = PowerGridOverlay::GetFaceSpoke(Center, HexSize, Face);
		const FVector2D Towards = UHhGridFunctions::HexAxialCoordToPoint(HexFaces::GetNeighborOffset(Face), 1.f).GetSafeNormal();
		TestTrue(FString::Printf(TEXT("Spoke %d starts in the centre"), Index), Spoke.Key.Equals(Center, 0.01f));
		TestTrue(FString::Printf(TEXT("Spoke %d points to the neighbour behind the face"), Index),
			(Spoke.Value - Spoke.Key).GetSafeNormal().Equals(Towards, 0.01f));
		TestEqual(FString::Printf(TEXT("Spoke %d reaches the edge"), Index), static_cast<float>((Spoke.Value - Spoke.Key).Size()), Apothem, 0.01f);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootUnderlayGridTest, "TypicalShooter.Loot.UI.Underlay.GridPushesFaces", LootUnderlayTest::Flags)
bool FLootUnderlayGridTest::RunTest(const FString& Parameters)
{
	using namespace LootUnderlayTest;
	InventoryCoreTest::FFixture F;
	F.Inventory->AddItem(InventoryCoreTest::NamedManifest(TEXT("Conduit")), CellA);
	F.Inventory->AddItem(InventoryCoreTest::NamedManifest(TEXT("Medkit")), CellB);
	const TStrongObjectPtr<UItemsContainer_Presenter> Presenter(UItemsContainer_Presenter::CreatePresenter(F.Inventory));
	const TSharedRef<SInventoryGrid> Grid = SNew(SInventoryGrid);
	Presenter->SetView(Grid);

	const int32 Bent = HexFaces::Bit(EHexFace::Right) | HexFaces::Bit(EHexFace::BottomLeft);
	Grid->SetPowerGrid(DisplayWith(CellA, Bent, true));
	const TOptional<FItemUnderlay> Underlay = Grid->GetItemViewAt(CellA)->GetUnderlay();
	if (TestTrue(TEXT("Power item gets an underlay"), Underlay.IsSet()))
	{
		TestEqual(TEXT("Underlay shows the display's (rotated) faces"), Underlay->OpenFaces, Bent);
		TestTrue(TEXT("Underlay knows the item is powered"), Underlay->bPowered);
	}
	TestFalse(TEXT("Item outside the power grid has no underlay"), Grid->GetItemViewAt(CellB)->GetUnderlay().IsSet());

	F.Inventory->AddItem(InventoryCoreTest::NamedManifest(TEXT("Late")), CellC);
	Grid->SetPowerGrid(DisplayWith(CellC, HexFaces::AllFaces, false));
	TestFalse(TEXT("An item that left the display loses its underlay"), Grid->GetItemViewAt(CellA)->GetUnderlay().IsSet());
	TestTrue(TEXT("The new cell gets its underlay"), Grid->GetItemViewAt(CellC)->GetUnderlay().IsSet());

	F.Inventory->AddItem(InventoryCoreTest::NamedManifest(TEXT("Even later")), FHhAxialCoord(-1, 0));
	Grid->SetPowerGrid(DisplayWith(FHhAxialCoord(-1, 0), HexFaces::AllFaces, false));
	F.Inventory->RemoveItem(F.Inventory->GetItemAt(FHhAxialCoord(-1, 0)));
	F.Inventory->AddItem(InventoryCoreTest::NamedManifest(TEXT("Replacement")), FHhAxialCoord(-1, 0));
	TestTrue(TEXT("An item view added under a known cell gets its underlay at once"),
		Grid->GetItemViewAt(FHhAxialCoord(-1, 0))->GetUnderlay().IsSet());
	Presenter->SetView(nullptr);
	return true;
}

#endif
