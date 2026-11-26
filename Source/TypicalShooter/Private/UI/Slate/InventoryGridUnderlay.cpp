// SInventoryGrid: item underlays from the power grid display (LootGeneration R11).

#include "UI/Slate/InventoryGrid.h"

#include "UI/Slate/InventoryGridItem.h"

void SInventoryGrid::ApplyUnderlays()
{
	for (const TPair<FGuid, TSharedRef<SInventoryGridItem>>& Pair : ItemWidgets)
	{
		const TOptional<FHhAxialCoord> Location = GetSlotLocation(Pair.Key);
		const FPowerCellDisplay* Cell = Location ? PowerGrid.FindCell(*Location) : nullptr;
		Pair.Value->SetUnderlay(Cell ? TOptional<FItemUnderlay>(FItemUnderlay{Cell->OpenFaces, Cell->bPowered}) : TOptional<FItemUnderlay>());
	}
}
