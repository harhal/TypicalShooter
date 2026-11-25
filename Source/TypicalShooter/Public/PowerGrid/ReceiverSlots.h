// Receiver slots: three fixed inventory cells where a powered receiver grants its ability (power grid R4, R4a).

#pragma once

#include "CoreMinimal.h"
#include "HhAxialCoord.h"
#include "ReceiverSlots.generated.h"

/** Which action a receiver slot triggers (R5). */
UENUM(BlueprintType)
enum class EReceiverSlot : uint8
{
	/** Top-left (0,-1): primary attack button, fires at the aim point. */
	Primary,
	/** Bottom-left (-1,1): alternate attack button, fires at the aim point. */
	Alternate,
	/** Right (1,0): jump button while airborne, fires downwards. */
	Air,
};

namespace ReceiverSlots
{
	/** Every slot, in enum order. */
	TYPICALSHOOTER_API TConstArrayView<EReceiverSlot> GetAll();

	/** Cell of Slot: every other cell of the first ring, a triangle pointing right on the pointy-top grid. */
	TYPICALSHOOTER_API FHhAxialCoord GetLocation(EReceiverSlot Slot);

	/** Slot at Location, unset for ordinary cells. Any item may lie in a slot (R4a). */
	TYPICALSHOOTER_API TOptional<EReceiverSlot> GetSlotAt(const FHhAxialCoord& Location);

	inline bool IsSlotCell(const FHhAxialCoord& Location) { return GetSlotAt(Location).IsSet(); }
}
