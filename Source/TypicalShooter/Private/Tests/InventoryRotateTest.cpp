// Rotating items in an inventory: server-only operation reported through the layout event (power grid R3).

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "InventoryLayout.h"
#include "Tests/InventoryTestFixture.h"

namespace InventoryRotateTest
{
	struct FRecorder
	{
		TArray<FInventoryLayoutChange> Events;

		explicit FRecorder(UInventoryComponent* Inventory)
		{
			Inventory->GetLayoutChangedDelegate().AddLambda([this](UInventoryComponent*, const FInventoryLayoutChange& Change)
			{
				Events.Add(Change);
			});
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryLayoutDiffRotationsTest, "TypicalShooter.Inventory.Layout.DiffRotations", InventoryCoreTest::Flags)
bool FInventoryLayoutDiffRotationsTest::RunTest(const FString& Parameters)
{
	InventoryCoreTest::FFixture F;
	UInventoryItem* Kept = NewObject<UInventoryItem>(F.Inventory);
	UInventoryItem* Rotated = NewObject<UInventoryItem>(F.Inventory);
	UInventoryItem* Removed = NewObject<UInventoryItem>(F.Inventory);
	UInventoryItem* Added = NewObject<UInventoryItem>(F.Inventory);

	const InventoryLayout::FRotations Before = { {Kept, 1}, {Rotated, 0}, {Removed, 2} };
	const InventoryLayout::FRotations After = { {Kept, 1}, {Rotated, 4}, {Added, 3} };
	const TArray<FInventoryItemRotationChange> Changes = InventoryLayout::DiffRotations(Before, After);

	if (TestEqual(TEXT("Only the item placed before and after with another rotation is reported"), Changes.Num(), 1))
	{
		TestTrue(TEXT("Rotated item"), Changes[0].Item == Rotated);
		TestEqual(TEXT("Previous rotation"), Changes[0].PrevRotation, 0);
		TestEqual(TEXT("New rotation"), Changes[0].Rotation, 4);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryRotateItemTest, "TypicalShooter.Inventory.Core.RotateItem", InventoryCoreTest::Flags)
bool FInventoryRotateItemTest::RunTest(const FString& Parameters)
{
	using namespace InventoryCoreTest;
	FFixture F;
	UInventoryItem* Item = F.Add(CellA);
	UInventoryItem* Neighbor = F.Add(CellB);
	InventoryRotateTest::FRecorder Recorder(F.Inventory);

	TestTrue(TEXT("Rotate a placed item"), F.Inventory->RotateItem(Item));
	TestEqual(TEXT("One clockwise step by default"), Item->GetRotation(), 1);
	if (TestEqual(TEXT("One layout event"), Recorder.Events.Num(), 1))
	{
		const FInventoryLayoutChange& Change = Recorder.Events[0];
		TestTrue(TEXT("No placement changes"), Change.Placements.IsEmpty());
		TestTrue(TEXT("No neighbour changes"), Change.Neighbors.IsEmpty());
		const FInventoryItemRotationChange* Rotation = Change.FindRotation(Item);
		if (TestNotNull(TEXT("Rotation is reported"), Rotation))
		{
			TestEqual(TEXT("From"), Rotation->PrevRotation, 0);
			TestEqual(TEXT("To"), Rotation->Rotation, 1);
		}
		TestNull(TEXT("The neighbour did not rotate"), Change.FindRotation(Neighbor));
	}

	TestTrue(TEXT("Rotate by several steps"), F.Inventory->RotateItem(Item, 5));
	TestEqual(TEXT("Rotation wraps"), Item->GetRotation(), 0);
	TestEqual(TEXT("Second event"), Recorder.Events.Num(), 2);

	TestTrue(TEXT("A full turn is accepted"), F.Inventory->RotateItem(Item, 6));
	TestEqual(TEXT("... but changes nothing and reports nothing"), Recorder.Events.Num(), 2);

	UInventoryComponent* Other = F.MakeInventory();
	UInventoryItem* Foreign = Other->AddItem(FInventoryItemManifest(), CellA);
	TestFalse(TEXT("Items of another inventory are rejected"), F.Inventory->RotateItem(Foreign));
	TestEqual(TEXT("Foreign item stays unrotated"), Foreign->GetRotation(), 0);
	TestFalse(TEXT("Null item is rejected"), F.Inventory->RotateItem(nullptr));
	TestEqual(TEXT("Rejected rotations report nothing"), Recorder.Events.Num(), 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryRotateNeedsAuthorityTest, "TypicalShooter.Inventory.Core.RotateNeedsAuthority", InventoryCoreTest::Flags)
bool FInventoryRotateNeedsAuthorityTest::RunTest(const FString& Parameters)
{
	InventoryCoreTest::FFixture F;
	UInventoryItem* Item = F.Add(InventoryCoreTest::CellA);
	F.Actor->SetRole(ROLE_SimulatedProxy);
	TestFalse(TEXT("Clients cannot rotate directly"), F.Inventory->RotateItem(Item));
	TestEqual(TEXT("Rotation unchanged"), Item->GetRotation(), 0);
	return true;
}

#endif
