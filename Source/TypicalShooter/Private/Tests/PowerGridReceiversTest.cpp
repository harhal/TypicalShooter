// Which receiver works in each receiver slot: a receiver in the slot cell fed with at least its required power
// (power grid R4, R4a, R6, R6a). Bare game world, server side.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "PowerGrid/PowerGridComponent.h"
#include "PowerGrid/PowerGridFragment.h"
#include "PowerGrid/ReceiverSlots.h"
#include "Tests/InventoryTestFixture.h"

namespace PowerGridReceiversTest
{
	FInventoryItemManifest Manifest(const EPowerItemType Type, const int32 Power = 1, const int32 RequiredPower = 1)
	{
		FPowerGrid_ItemFragment Fragment;
		Fragment.ItemType = Type;
		Fragment.Source.Power = Power;
		Fragment.Receiver.RequiredPower = RequiredPower;
		FInventoryItemManifest Result;
		Result.Fragments.Add(TInstancedStruct<FItemFragment>::Make<FPowerGrid_ItemFragment>(Fragment));
		return Result;
	}

	UPowerGridComponent* AddGrid(AActor* Actor)
	{
		UPowerGridComponent* Grid = NewObject<UPowerGridComponent>(Actor, TEXT("PowerGrid"));
		Grid->RegisterComponent();
		return Grid;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerGridActiveReceiverTest, "TypicalShooter.PowerGrid.Receivers.ActiveInSlot", InventoryCoreTest::Flags)
bool FPowerGridActiveReceiverTest::RunTest(const FString& Parameters)
{
	using namespace PowerGridReceiversTest;
	InventoryCoreTest::FFixture F;
	UPowerGridComponent* Grid = AddGrid(F.Actor);
	const FHhAxialCoord PrimaryCell = ReceiverSlots::GetLocation(EReceiverSlot::Primary);
	const FHhAxialCoord AlternateCell = ReceiverSlots::GetLocation(EReceiverSlot::Alternate);
	const FHhAxialCoord AirCell = ReceiverSlots::GetLocation(EReceiverSlot::Air);

	F.Inventory->AddItem(Manifest(EPowerItemType::Source, 1), FHhAxialCoord(0, 0));
	UInventoryItem* Rifle = F.Inventory->AddItem(Manifest(EPowerItemType::Receiver), PrimaryCell);
	UInventoryItem* Hungry = F.Inventory->AddItem(Manifest(EPowerItemType::Receiver, 1, 2), AlternateCell);
	F.Inventory->AddItem(Manifest(EPowerItemType::Connector), AirCell);
	UInventoryItem* Stored = F.Inventory->AddItem(Manifest(EPowerItemType::Receiver), FHhAxialCoord(1, -1));

	TestTrue(TEXT("Powered receiver in the primary slot works (R5, R6)"), Grid->GetActiveReceiver(EReceiverSlot::Primary) == Rifle);
	TestTrue(TEXT("Underpowered receiver lies in the alternate slot"), Grid->GetSlotReceiver(EReceiverSlot::Alternate) == Hungry);
	TestNull(TEXT("... but does not work: power below its requirement (R6)"), Grid->GetActiveReceiver(EReceiverSlot::Alternate));
	TestNull(TEXT("A connector in the air slot is no receiver (R4a)"), Grid->GetSlotReceiver(EReceiverSlot::Air));
	TestNull(TEXT("No action without a receiver (R6a)"), Grid->GetActiveReceiver(EReceiverSlot::Air));
	TestTrue(TEXT("A powered receiver outside the slots is only stored"), Grid->IsItemPowered(Stored));
	TestFalse(TEXT("... and has no slot"), Grid->GetItemSlot(Stored).IsSet());
	TestTrue(TEXT("Slot of a receiver in a slot cell"), Grid->GetItemSlot(Rifle) == TOptional<EReceiverSlot>(EReceiverSlot::Primary));

	TestTrue(TEXT("Move the rifle out of its slot"), F.Inventory->MoveItem(Rifle, FHhAxialCoord(0, 1)));
	TestNull(TEXT("Primary slot is empty now"), Grid->GetActiveReceiver(EReceiverSlot::Primary));
	return true;
}

#endif
