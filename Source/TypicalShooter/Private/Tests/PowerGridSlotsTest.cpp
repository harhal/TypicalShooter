// Receiver slots of an inventory: fixed cells of the first ring, which slot a cell is (power grid R4, R4a).

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "HhGridFunctions.h"
#include "PowerGrid/ReceiverSlots.h"

namespace PowerGridSlotsTest
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerGridSlotsLocationsTest, "TypicalShooter.PowerGrid.Slots.Locations", PowerGridSlotsTest::Flags)
bool FPowerGridSlotsLocationsTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Primary attack: top-left (0,-1)"), ReceiverSlots::GetLocation(EReceiverSlot::Primary), FHhAxialCoord(0, -1));
	TestEqual(TEXT("Alternate attack: bottom-left (-1,1)"), ReceiverSlots::GetLocation(EReceiverSlot::Alternate), FHhAxialCoord(-1, 1));
	TestEqual(TEXT("Air attack: right (1,0)"), ReceiverSlots::GetLocation(EReceiverSlot::Air), FHhAxialCoord(1, 0));

	const TConstArrayView<EReceiverSlot> All = ReceiverSlots::GetAll();
	TestEqual(TEXT("Three slots"), All.Num(), 3);
	const FHhAxialCoord Centre(0, 0);
	for (const EReceiverSlot SlotA : All)
	{
		const FHhAxialCoord A = ReceiverSlots::GetLocation(SlotA);
		TestEqual(TEXT("Slot lies in the first ring"), UHhGridFunctions::GetHexDistance(Centre, A), 1);
		for (const EReceiverSlot SlotB : All)
		{
			if (SlotA != SlotB)
			{
				TestEqual(TEXT("Slots are every other cell of the ring"),
					UHhGridFunctions::GetHexDistance(A, ReceiverSlots::GetLocation(SlotB)), 2);
			}
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerGridSlotsAtCellTest, "TypicalShooter.PowerGrid.Slots.SlotAtCell", PowerGridSlotsTest::Flags)
bool FPowerGridSlotsAtCellTest::RunTest(const FString& Parameters)
{
	for (const EReceiverSlot Slot : ReceiverSlots::GetAll())
	{
		const TOptional<EReceiverSlot> Found = ReceiverSlots::GetSlotAt(ReceiverSlots::GetLocation(Slot));
		TestTrue(TEXT("Slot cell maps back to its slot"), Found.IsSet() && Found.GetValue() == Slot);
		TestTrue(TEXT("IsSlotCell on a slot cell"), ReceiverSlots::IsSlotCell(ReceiverSlots::GetLocation(Slot)));
	}

	const FHhAxialCoord NotSlots[] = {
		FHhAxialCoord(0, 0), FHhAxialCoord(1, -1), FHhAxialCoord(-1, 0), FHhAxialCoord(0, 1), // centre + other ring cells
		FHhAxialCoord(2, 0), FHhAxialCoord(0, -2), FHhAxialCoord(-2, 2),                    // second ring
	};
	for (const FHhAxialCoord& Cell : NotSlots)
	{
		TestFalse(FString::Printf(TEXT("(%d,%d) is not a slot"), Cell.CellCoord.X, Cell.CellCoord.Y),
			ReceiverSlots::GetSlotAt(Cell).IsSet());
	}
	return true;
}

#endif
