// Power grid solver: a pure function from placed power items to connections, incoming power and receiver
// connectivity (power grid R2a, R2b, R7, R8, R10-R12a, R14). Nodes are referred to by their index in the input.

#pragma once

#include "CoreMinimal.h"
#include "HhAxialCoord.h"
#include "PowerGrid/PowerGridFragment.h"

/** One placed power item. */
struct FPowerGridNode
{
	FHhAxialCoord Location;
	EPowerItemType Type = EPowerItemType::Connector;
	/** Open faces with the item's rotation already applied. */
	int32 OpenFaces = HexFaces::AllFaces;
	/** Sources only. */
	int32 SourcePower = 0;
	/** Sources only: false = out of ammo, gives nothing. */
	bool bSourceWorking = true;
};

/** Directed connection: power may flow From -> To (both facing faces open, From outputs, To accepts). */
struct FPowerGridEdge
{
	int32 From = INDEX_NONE;
	int32 To = INDEX_NONE;
	/** Power actually flows: From is a working source or is fed by one. */
	bool bPowered = false;
};

struct FPowerGridNodeResult
{
	/** Sum of the distinct working sources reaching the node; a source's own power if it works, else 0 (R12a, R13). */
	int32 Power = 0;
	/** Indices of the distinct working sources counted in Power. */
	TArray<int32> Sources;
	/** A working source or fed by one. */
	bool bPowered = false;
	/** Receivers only: itself + nodes on powered paths from a working source to this receiver (R14). */
	TArray<int32> Connected;
};

struct TYPICALSHOOTER_API FPowerGridSolution
{
	/** Parallel to the input nodes. */
	TArray<FPowerGridNodeResult> Nodes;
	TArray<FPowerGridEdge> Edges;

	const FPowerGridEdge* FindEdge(int32 From, int32 To) const;
};

namespace PowerGridSolver
{
	TYPICALSHOOTER_API FPowerGridSolution Solve(TConstArrayView<FPowerGridNode> Nodes);
}
