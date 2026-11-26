// Geometry and labels of the power grid drawn in SInventoryGrid (power grid R16): open faces as underlay spokes of the
// item cells, links between cell centres, receiver slot marks and power / clip labels.

#pragma once

#include "CoreMinimal.h"
#include "HhAxialCoord.h"
#include "PowerGrid/HexFaces.h"
#include "PowerGrid/ReceiverSlots.h"
#include "UI/PowerGrid/PowerGridDisplay.h"

namespace PowerGridOverlay
{
	inline const FLinearColor PoweredColor(1.f, 0.8f, 0.15f, 1.f);
	inline const FLinearColor UnpoweredColor(0.55f, 0.55f, 0.6f, 0.8f);

	/** Centre of Coord in a grid widget of LocalSize whose (0,0) cell sits in the middle (like SHhGridPanel). */
	TYPICALSHOOTER_API FVector2D GetCellCenter(const FHhAxialCoord& Coord, float HexSize, const FVector2D& LocalSize);

	/** Spoke of the item underlay (LootGeneration R11): from Center to the middle of Face's edge. */
	TYPICALSHOOTER_API TPair<FVector2D, FVector2D> GetFaceSpoke(const FVector2D& Center, float HexSize, EHexFace Face);

	/** Power of the item: "2"; a receiver "fed/required"; an ammo source adds its clip "2 [1/4]". */
	TYPICALSHOOTER_API FText GetPowerLabel(const FPowerCellDisplay& Cell);

	/** Mark of a receiver slot cell (empty for other cells). */
	TYPICALSHOOTER_API FText GetSlotLabel(const FHhAxialCoord& Location);
}
