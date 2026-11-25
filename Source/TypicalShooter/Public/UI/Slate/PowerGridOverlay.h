// Geometry and labels of the power grid drawn over SInventoryGrid (power grid R16): open faces as short strokes along
// the hex edges, links between cell centres, receiver slot marks and power / clip labels.

#pragma once

#include "CoreMinimal.h"
#include "HhAxialCoord.h"
#include "PowerGrid/HexFaces.h"
#include "PowerGrid/ReceiverSlots.h"
#include "UI/PowerGrid/PowerGridDisplay.h"

namespace PowerGridOverlay
{
	/** Centre of Coord in a grid widget of LocalSize whose (0,0) cell sits in the middle (like SHhGridPanel). */
	TYPICALSHOOTER_API FVector2D GetCellCenter(const FHhAxialCoord& Coord, float HexSize, const FVector2D& LocalSize);

	/** Stroke marking an open Face of the pointy-top hex at Center: along that edge, slightly inside the cell. */
	TYPICALSHOOTER_API TPair<FVector2D, FVector2D> GetFaceSegment(const FVector2D& Center, float HexSize, EHexFace Face);

	/** Power of the item: "2"; a receiver "fed/required"; an ammo source adds its clip "2 [1/4]". */
	TYPICALSHOOTER_API FText GetPowerLabel(const FPowerCellDisplay& Cell);

	/** Mark of a receiver slot cell (empty for other cells). */
	TYPICALSHOOTER_API FText GetSlotLabel(const FHhAxialCoord& Location);
}
