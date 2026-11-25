// What the inventory UI shows about the power grid (power grid R16), built from a solved UPowerGridComponent:
// per item - faces with rotation, power, clip; grid links; whether the receiver slots are marked; and the info panel
// description of one item (type, power / required power, effect steps with the reached ones flagged).

#pragma once

#include "CoreMinimal.h"
#include "HhAxialCoord.h"
#include "PowerGrid/PowerGridFragment.h"

class UInventoryItem;
class UPowerGridComponent;

/** One power item on the grid. */
struct TYPICALSHOOTER_API FPowerCellDisplay
{
	FHhAxialCoord Location;
	EPowerItemType Type = EPowerItemType::Connector;
	/** Open faces with the item's rotation (HexFaces bit mask). */
	int32 OpenFaces = 0;
	/** Power fed to the item (a source: its own power while it works). */
	int32 Power = 0;
	bool bPowered = false;
	/** Receivers: power needed to work. */
	int32 RequiredPower = 0;
	/** Sources needing ammo: clip / capacity. */
	TOptional<FIntPoint> Clip;
};

/** A connection between two neighbouring items (both facing faces open), From gives power to To. */
struct TYPICALSHOOTER_API FPowerLinkDisplay
{
	FHhAxialCoord From;
	FHhAxialCoord To;
	bool bPowered = false;
};

struct TYPICALSHOOTER_API FPowerGridDisplay
{
	TArray<FPowerCellDisplay> Cells;
	TArray<FPowerLinkDisplay> Links;
	/** The receiver slots mean something here (the owner turns receivers into abilities). */
	bool bShowSlots = false;

	const FPowerCellDisplay* FindCell(const FHhAxialCoord& Location) const;
	bool operator==(const FPowerGridDisplay& Other) const;
};

/** One effect step of an item for the info panel. */
struct TYPICALSHOOTER_API FPowerStepInfo
{
	FText Text;
	bool bReached = false;
};

/** Power grid part of the item info panel; empty when the item has neither a power role nor effects. */
struct TYPICALSHOOTER_API FItemPowerInfo
{
	/** "Source" / "Receiver" / "Connector", empty without a power role. */
	FText TypeText;
	/** Power line: fed power, required power, clip. */
	FText PowerText;
	TArray<FPowerStepInfo> Steps;

	bool IsEmpty() const { return TypeText.IsEmpty() && Steps.IsEmpty(); }
};

namespace PowerGridDisplay
{
	TYPICALSHOOTER_API FPowerGridDisplay Build(const UPowerGridComponent& Grid);

	/** Info of Item; Grid = the power grid of the item's inventory (null = none: power 0). */
	TYPICALSHOOTER_API FItemPowerInfo DescribeItem(const UInventoryItem& Item, const UPowerGridComponent* Grid);

	/** The power grid of the inventory Item lies in, if any. */
	TYPICALSHOOTER_API UPowerGridComponent* FindGridOf(const UInventoryItem& Item);
}
