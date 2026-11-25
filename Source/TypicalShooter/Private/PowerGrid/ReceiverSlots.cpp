#include "PowerGrid/ReceiverSlots.h"

namespace ReceiverSlots
{
	static const EReceiverSlot AllSlots[] = { EReceiverSlot::Primary, EReceiverSlot::Alternate, EReceiverSlot::Air };

	TConstArrayView<EReceiverSlot> GetAll()
	{
		return AllSlots;
	}

	FHhAxialCoord GetLocation(const EReceiverSlot Slot)
	{
		switch (Slot)
		{
		case EReceiverSlot::Primary:   return FHhAxialCoord(0, -1);
		case EReceiverSlot::Alternate: return FHhAxialCoord(-1, 1);
		case EReceiverSlot::Air:       return FHhAxialCoord(1, 0);
		}
		checkNoEntry();
		return FHhAxialCoord(0, 0);
	}

	TOptional<EReceiverSlot> GetSlotAt(const FHhAxialCoord& Location)
	{
		for (const EReceiverSlot Slot : AllSlots)
		{
			if (GetLocation(Slot) == Location)
			{
				return Slot;
			}
		}
		return {};
	}
}
