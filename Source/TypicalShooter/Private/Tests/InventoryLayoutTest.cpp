// Inventory layout change notifications: one consistent event per atomic operation, with placement and
// neighbour (adjacency) changes. Pure diff + server-side component behaviour (bare game world, no PIE).

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "HhGridFunctions.h"
#include "InventoryLayout.h"
#include "Tests/InventoryTestFixture.h"

namespace InventoryLayoutTest
{
	/** Records every layout event of an inventory. */
	struct FRecorder
	{
		TArray<FInventoryLayoutChange> Events;
		/** Item counts seen by listeners at broadcast time (checks the inventory is consistent by then). */
		TArray<int32> NumItemsAtBroadcast;

		explicit FRecorder(UInventoryComponent* Inventory)
		{
			Inventory->GetLayoutChangedDelegate().AddLambda([this](UInventoryComponent* Source, const FInventoryLayoutChange& Change)
			{
				Events.Add(Change);
				NumItemsAtBroadcast.Add(Source->GetNumItems());
			});
		}

		/** The latest event, or an empty change if nothing was broadcast (keeps failing tests from crashing). */
		const FInventoryLayoutChange& Last() const
		{
			static const FInventoryLayoutChange Empty;
			return Events.IsEmpty() ? Empty : Events.Last();
		}
	};

	bool HasNeighbor(const TArray<TObjectPtr<UInventoryItem>>& Items, const UInventoryItem* Item)
	{
		return Items.Contains(Item);
	}

	/** Makes distinct dummy items (outer = the fixture inventory) for pure diff tests. */
	UInventoryItem* MakeLooseItem(UObject* Outer)
	{
		return NewObject<UInventoryItem>(Outer);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryLayoutNeighborLocationsTest, "TypicalShooter.Inventory.Layout.NeighborLocations", InventoryCoreTest::Flags)
bool FInventoryLayoutNeighborLocationsTest::RunTest(const FString& Parameters)
{
	const FHhAxialCoord Centre(1, -1);
	const auto Neighbors = InventoryLayout::GetNeighborLocations(Centre);
	TestEqual(TEXT("Six neighbours"), Neighbors.Num(), 6);

	TSet<FHhAxialCoord> Unique;
	for (const FHhAxialCoord& Neighbor : Neighbors)
	{
		Unique.Add(Neighbor);
		TestEqual(TEXT("Each neighbour is one step away"), UHhGridFunctions::GetHexDistance(Centre, Neighbor), 1);
	}
	TestEqual(TEXT("Neighbours are distinct"), Unique.Num(), 6);
	TestTrue(TEXT("First offset points right"), InventoryLayout::GetNeighborOffsets()[0] == FHhAxialCoord(1, 0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryLayoutDiffTest, "TypicalShooter.Inventory.Layout.Diff", InventoryCoreTest::Flags)
bool FInventoryLayoutDiffTest::RunTest(const FString& Parameters)
{
	using namespace InventoryCoreTest;
	using namespace InventoryLayoutTest;
	FFixture F;
	UInventoryItem* A = MakeLooseItem(F.Inventory);
	UInventoryItem* B = MakeLooseItem(F.Inventory);
	UInventoryItem* C = MakeLooseItem(F.Inventory);
	UInventoryItem* D = MakeLooseItem(F.Inventory);

	// A(0,0) - B(1,0) adjacent; C far away at (-2,0); D added next to A.
	const InventoryLayout::FLayout Before = { {A, FHhAxialCoord(0, 0)}, {B, FHhAxialCoord(1, 0)}, {C, FHhAxialCoord(-2, 0)} };
	// B moves away from A (to (2,-2)), C is removed, D appears at (-1,0) next to A.
	const InventoryLayout::FLayout After = { {A, FHhAxialCoord(0, 0)}, {B, FHhAxialCoord(2, -2)}, {D, FHhAxialCoord(-1, 0)} };

	const FInventoryLayoutChange Change = InventoryLayout::Diff(Before, After);

	TestEqual(TEXT("Three placement changes (B moved, C removed, D added)"), Change.Placements.Num(), 3);
	TestNull(TEXT("A did not move"), Change.FindPlacement(A));
	if (const FInventoryItemPlacementChange* Moved = Change.FindPlacement(B); TestNotNull(TEXT("B reported"), Moved))
	{
		TestTrue(TEXT("B is a move"), Moved->IsMoved());
		TestTrue(TEXT("B's previous cell"), Moved->PrevLocation == FHhAxialCoord(1, 0));
		TestTrue(TEXT("B's new cell"), Moved->Location == FHhAxialCoord(2, -2));
	}
	if (const FInventoryItemPlacementChange* Removed = Change.FindPlacement(C); TestNotNull(TEXT("C reported"), Removed))
	{
		TestTrue(TEXT("C is a removal"), Removed->IsRemoved());
		TestTrue(TEXT("C's previous cell"), Removed->PrevLocation == FHhAxialCoord(-2, 0));
	}
	if (const FInventoryItemPlacementChange* Added = Change.FindPlacement(D); TestNotNull(TEXT("D reported"), Added))
	{
		TestTrue(TEXT("D is an addition"), Added->IsAdded());
		TestTrue(TEXT("D's cell"), Added->Location == FHhAxialCoord(-1, 0));
	}

	// Neighbours: A lost B and gained D; B lost A; D gained A. C never had neighbours -> not reported.
	const FInventoryItemNeighborsChange* NA = Change.FindNeighbors(A);
	if (TestNotNull(TEXT("A's neighbours changed"), NA))
	{
		TestTrue(TEXT("A gained D"), HasNeighbor(NA->GainedNeighbors, D));
		TestTrue(TEXT("A lost B"), HasNeighbor(NA->LostNeighbors, B));
		TestEqual(TEXT("A gained only D"), NA->GainedNeighbors.Num(), 1);
		TestEqual(TEXT("A lost only B"), NA->LostNeighbors.Num(), 1);
	}
	const FInventoryItemNeighborsChange* NB = Change.FindNeighbors(B);
	if (TestNotNull(TEXT("B's neighbours changed"), NB))
	{
		TestTrue(TEXT("B lost A"), HasNeighbor(NB->LostNeighbors, A));
		TestEqual(TEXT("B gained nobody"), NB->GainedNeighbors.Num(), 0);
	}
	const FInventoryItemNeighborsChange* ND = Change.FindNeighbors(D);
	if (TestNotNull(TEXT("D's neighbours changed"), ND))
	{
		TestTrue(TEXT("D gained A"), HasNeighbor(ND->GainedNeighbors, A));
	}
	TestNull(TEXT("Isolated C has no neighbour change"), Change.FindNeighbors(C));

	TestTrue(TEXT("Identical layouts give an empty change"), InventoryLayout::Diff(After, After).IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryLayoutMoveEventTest, "TypicalShooter.Inventory.Layout.MoveBroadcastsOnce", InventoryCoreTest::Flags)
bool FInventoryLayoutMoveEventTest::RunTest(const FString& Parameters)
{
	using namespace InventoryCoreTest;
	using namespace InventoryLayoutTest;
	FFixture F;
	FRecorder R(F.Inventory);

	UInventoryItem* Medkit = F.Add(CellA);
	TestEqual(TEXT("Add: one event"), R.Events.Num(), 1);
	TestTrue(TEXT("Add: reported as added"), R.Last().FindPlacement(Medkit) && R.Last().FindPlacement(Medkit)->IsAdded());

	UInventoryItem* Ammo = F.Add(CellB);
	TestEqual(TEXT("Second add: one more event"), R.Events.Num(), 2);
	const FInventoryItemNeighborsChange* MedkitNeighbors = R.Last().FindNeighbors(Medkit);
	TestTrue(TEXT("Adding next to Medkit gives Medkit a neighbour"), MedkitNeighbors && HasNeighbor(MedkitNeighbors->GainedNeighbors, Ammo));

	TestTrue(TEXT("Move away"), F.Inventory->MoveItem(Ammo, FHhAxialCoord(2, -2)));
	TestEqual(TEXT("Move: one event"), R.Events.Num(), 3);
	TestEqual(TEXT("Move: one placement change"), R.Last().Placements.Num(), 1);
	TestTrue(TEXT("Move: reported as moved"), R.Last().FindPlacement(Ammo) && R.Last().FindPlacement(Ammo)->IsMoved());
	MedkitNeighbors = R.Last().FindNeighbors(Medkit);
	TestTrue(TEXT("Moving away: Medkit lost its neighbour"), MedkitNeighbors && HasNeighbor(MedkitNeighbors->LostNeighbors, Ammo));

	TestTrue(TEXT("Move within the same neighbourhood-less area"), F.Inventory->MoveItem(Ammo, FHhAxialCoord(2, -1)));
	TestNull(TEXT("No neighbour change when nothing became adjacent"), R.Last().FindNeighbors(Medkit));

	const int32 NumEvents = R.Events.Num();
	TestFalse(TEXT("Move onto an occupied cell fails"), F.Inventory->MoveItem(Ammo, CellA));
	TestTrue(TEXT("Move to the same cell succeeds"), F.Inventory->MoveItem(Ammo, FHhAxialCoord(2, -1)));
	TestEqual(TEXT("Failed / no-op operations broadcast nothing"), R.Events.Num(), NumEvents);

	F.Inventory->RemoveItem(Medkit);
	TestTrue(TEXT("Remove: reported as removed"), R.Last().FindPlacement(Medkit) && R.Last().FindPlacement(Medkit)->IsRemoved());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryLayoutSwapEventTest, "TypicalShooter.Inventory.Layout.SwapIsOneEvent", InventoryCoreTest::Flags)
bool FInventoryLayoutSwapEventTest::RunTest(const FString& Parameters)
{
	using namespace InventoryCoreTest;
	using namespace InventoryLayoutTest;
	FFixture F;
	UInventoryItem* Medkit = F.Add(CellA);
	UInventoryItem* Ammo = F.Add(FHhAxialCoord(2, -2));
	UInventoryItem* Grenade = F.Add(FHhAxialCoord(-1, 0)); // neighbour of CellA only
	FRecorder R(F.Inventory);

	TestTrue(TEXT("Swap through TransferItem"), F.Inventory->TransferItem(Medkit, F.Inventory, FHhAxialCoord(2, -2)));
	TestEqual(TEXT("A swap is a single layout event"), R.Events.Num(), 1);
	if (R.Events.Num() != 1)
	{
		return false;
	}
	const FInventoryLayoutChange& Change = R.Last();
	TestEqual(TEXT("Both items moved"), Change.Placements.Num(), 2);
	TestTrue(TEXT("Medkit moved to Ammo's cell"), Change.FindPlacement(Medkit) && Change.FindPlacement(Medkit)->Location == FHhAxialCoord(2, -2));
	TestTrue(TEXT("Ammo moved to Medkit's cell"), Change.FindPlacement(Ammo) && Change.FindPlacement(Ammo)->Location == CellA);

	const FInventoryItemNeighborsChange* GrenadeNeighbors = Change.FindNeighbors(Grenade);
	if (TestNotNull(TEXT("Grenade's neighbour changed"), GrenadeNeighbors))
	{
		TestTrue(TEXT("Grenade lost Medkit"), HasNeighbor(GrenadeNeighbors->LostNeighbors, Medkit));
		TestTrue(TEXT("Grenade gained Ammo"), HasNeighbor(GrenadeNeighbors->GainedNeighbors, Ammo));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryLayoutTransferEventTest, "TypicalShooter.Inventory.Layout.TransferBetweenInventories", InventoryCoreTest::Flags)
bool FInventoryLayoutTransferEventTest::RunTest(const FString& Parameters)
{
	using namespace InventoryCoreTest;
	using namespace InventoryLayoutTest;
	FFixture F;
	UInventoryComponent* Player = F.MakeInventory(1);
	UInventoryItem* Medkit = F.Inventory->AddItem(NamedManifest(TEXT("Medkit")), CellA);
	UInventoryItem* Scope = Player->AddItem(NamedManifest(TEXT("Scope")), CellB);
	FRecorder Source(F.Inventory);
	FRecorder Target(Player);

	// Swap across inventories: remove + add on both sides, but each side reports one consistent event.
	TestTrue(TEXT("Swap between inventories"), F.Inventory->TransferItem(Medkit, Player, CellB));
	TestEqual(TEXT("Source: one event"), Source.Events.Num(), 1);
	TestEqual(TEXT("Target: one event"), Target.Events.Num(), 1);
	if (Source.Events.Num() != 1 || Target.Events.Num() != 1)
	{
		return false;
	}
	TestEqual(TEXT("Source is consistent (1 item) when it broadcasts"), Source.NumItemsAtBroadcast[0], 1);
	TestEqual(TEXT("Target is consistent (1 item) when it broadcasts"), Target.NumItemsAtBroadcast[0], 1);

	TestTrue(TEXT("Source: Medkit removed"), Source.Last().FindPlacement(Medkit) && Source.Last().FindPlacement(Medkit)->IsRemoved());
	UInventoryItem* ScopeInSource = FFixture::ItemAt(F.Inventory, CellA);
	TestTrue(TEXT("Source: Scope added at Medkit's cell"),
		ScopeInSource && Source.Last().FindPlacement(ScopeInSource) && Source.Last().FindPlacement(ScopeInSource)->IsAdded());
	TestTrue(TEXT("Target: Scope removed"), Target.Last().FindPlacement(Scope) && Target.Last().FindPlacement(Scope)->IsRemoved());
	TestEqual(TEXT("Target: two placement changes"), Target.Last().Placements.Num(), 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryLayoutBatchTest, "TypicalShooter.Inventory.Layout.ScopedBatch", InventoryCoreTest::Flags)
bool FInventoryLayoutBatchTest::RunTest(const FString& Parameters)
{
	using namespace InventoryCoreTest;
	using namespace InventoryLayoutTest;
	FFixture F;
	UInventoryComponent* Player = F.MakeInventory(2);
	F.Add(CellA);
	F.Add(CellB);
	F.Add(CellC);
	FRecorder Source(F.Inventory);
	FRecorder Target(Player);

	{
		UInventoryComponent::FScopedLayoutBatch SourceBatch(F.Inventory);
		UInventoryComponent::FScopedLayoutBatch TargetBatch(Player);
		for (const TPair<UInventoryItem*, FHhAxialCoord>& Placed : F.Inventory->GetPlacedItems())
		{
			F.Inventory->TransferItemToFreeLocation(Placed.Key, Player);
		}
		TestEqual(TEXT("Nothing is broadcast inside a batch"), Source.Events.Num() + Target.Events.Num(), 0);
	}

	TestEqual(TEXT("Source: one event for the whole batch"), Source.Events.Num(), 1);
	TestEqual(TEXT("Target: one event for the whole batch"), Target.Events.Num(), 1);
	if (Source.Events.Num() == 1 && Target.Events.Num() == 1)
	{
		TestEqual(TEXT("Source reports three removals"), Source.Last().Placements.Num(), 3);
		TestEqual(TEXT("Target reports three additions"), Target.Last().Placements.Num(), 3);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryLayoutQueriesTest, "TypicalShooter.Inventory.Layout.NeighborQueries", InventoryCoreTest::Flags)
bool FInventoryLayoutQueriesTest::RunTest(const FString& Parameters)
{
	using namespace InventoryCoreTest;
	FFixture F;
	UInventoryItem* Centre = F.Add(CellA);
	UInventoryItem* Right = F.Add(CellB);
	UInventoryItem* Below = F.Add(CellC);
	UInventoryItem* Far = F.Add(FHhAxialCoord(2, -2));

	const TArray<UInventoryItem*> CentreNeighbors = F.Inventory->GetItemNeighbors(Centre);
	TestEqual(TEXT("Centre has two neighbours"), CentreNeighbors.Num(), 2);
	TestTrue(TEXT("Right is a neighbour"), CentreNeighbors.Contains(Right));
	TestTrue(TEXT("Below is a neighbour"), CentreNeighbors.Contains(Below));
	TestFalse(TEXT("Far is not a neighbour"), CentreNeighbors.Contains(Far));
	TestEqual(TEXT("Far has no neighbours"), F.Inventory->GetItemNeighbors(Far).Num(), 0);
	TestEqual(TEXT("Items around an empty cell"), F.Inventory->GetNeighborItems(FHhAxialCoord(1, -1)).Num(), 3);
	TestEqual(TEXT("Unplaced item has no neighbours"), F.Inventory->GetItemNeighbors(nullptr).Num(), 0);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
