#include "PowerGrid/HexFaces.h"

namespace HexFaces
{
	FHhAxialCoord GetNeighborOffset(const EHexFace Face)
	{
		// Pointy-top axial layout (UHhGridFunctions::HexAxialCoordToPoint), screen Y down.
		static const FHhAxialCoord Offsets[Num] = {
			FHhAxialCoord(1, 0),   // Right
			FHhAxialCoord(0, 1),   // BottomRight
			FHhAxialCoord(-1, 1),  // BottomLeft
			FHhAxialCoord(-1, 0),  // Left
			FHhAxialCoord(0, -1),  // TopLeft
			FHhAxialCoord(1, -1),  // TopRight
		};
		return Offsets[static_cast<int32>(Face)];
	}

	bool FaceTowards(const FHhAxialCoord& From, const FHhAxialCoord& To, EHexFace& OutFace)
	{
		const FIntVector2 Delta = To.CellCoord - From.CellCoord;
		for (int32 Index = 0; Index < Num; ++Index)
		{
			if (GetNeighborOffset(FromIndex(Index)).CellCoord == Delta)
			{
				OutFace = FromIndex(Index);
				return true;
			}
		}
		return false;
	}

	int32 RotateMask(const int32 Mask, const int32 Steps)
	{
		const int32 Shift = ((Steps % Num) + Num) % Num;
		const int32 Faces = Mask & AllFaces;
		return ((Faces << Shift) | (Faces >> (Num - Shift))) & AllFaces;
	}
}
