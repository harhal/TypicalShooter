// Procedural underlay of an item cell: the item's open faces drawn as spokes from the centre (LootGeneration R11).
// Same for every item with the same (rotated) faces; the icon is drawn over it.

#pragma once

#include "CoreMinimal.h"

struct FItemUnderlay
{
	/** Open faces with the item's rotation (HexFaces bit mask). */
	int32 OpenFaces = 0;
	bool bPowered = false;

	bool operator==(const FItemUnderlay& Other) const { return OpenFaces == Other.OpenFaces && bPowered == Other.bPowered; }
};
