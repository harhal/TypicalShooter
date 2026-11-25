// The six faces of an inventory hex cell (pointy-top grid) and face masks with clockwise rotation (R2, R3).

#pragma once

#include "CoreMinimal.h"
#include "HhAxialCoord.h"
#include "HexFaces.generated.h"

/** Hex face, listed clockwise on screen starting from the right. Bit N of a face mask = face with index N. */
UENUM(BlueprintType, meta = (Bitflags))
enum class EHexFace : uint8
{
	Right,
	BottomRight,
	BottomLeft,
	Left,
	TopLeft,
	TopRight,
};

namespace HexFaces
{
	constexpr int32 Num = 6;
	constexpr int32 AllFaces = (1 << Num) - 1;

	inline EHexFace FromIndex(const int32 Index) { return static_cast<EHexFace>(((Index % Num) + Num) % Num); }
	inline int32 Bit(const EHexFace Face) { return 1 << static_cast<int32>(Face); }
	inline bool IsOpen(const int32 Mask, const EHexFace Face) { return (Mask & Bit(Face)) != 0; }
	inline EHexFace Opposite(const EHexFace Face) { return FromIndex(static_cast<int32>(Face) + Num / 2); }

	/** Axial offset of the neighbour behind Face. */
	TYPICALSHOOTER_API FHhAxialCoord GetNeighborOffset(EHexFace Face);

	/** Face of From that touches To; false if the cells are not adjacent. */
	TYPICALSHOOTER_API bool FaceTowards(const FHhAxialCoord& From, const FHhAxialCoord& To, EHexFace& OutFace);

	/** Mask rotated clockwise by Steps * 60 degrees (negative = counter-clockwise); bits above the six faces drop. */
	TYPICALSHOOTER_API int32 RotateMask(int32 Mask, int32 Steps);
}
