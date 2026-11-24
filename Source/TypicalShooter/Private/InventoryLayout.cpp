// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryLayout.h"

#include "InventoryItem.h"

const FInventoryItemPlacementChange* FInventoryLayoutChange::FindPlacement(const UInventoryItem* Item) const
{
	return Placements.FindByPredicate([Item](const FInventoryItemPlacementChange& Change) { return Change.Item == Item; });
}

const FInventoryItemNeighborsChange* FInventoryLayoutChange::FindNeighbors(const UInventoryItem* Item) const
{
	return Neighbors.FindByPredicate([Item](const FInventoryItemNeighborsChange& Change) { return Change.Item == Item; });
}

namespace InventoryLayout
{
	static const FHhAxialCoord NeighborOffsets[6] = {
		FHhAxialCoord(1, 0), FHhAxialCoord(1, -1), FHhAxialCoord(0, -1),
		FHhAxialCoord(-1, 0), FHhAxialCoord(-1, 1), FHhAxialCoord(0, 1)
	};

	TConstArrayView<FHhAxialCoord> GetNeighborOffsets()
	{
		return NeighborOffsets;
	}

	FHhAxialCoord Offset(const FHhAxialCoord& Location, const FHhAxialCoord& Direction)
	{
		return FHhAxialCoord(Location.CellCoord.X + Direction.CellCoord.X, Location.CellCoord.Y + Direction.CellCoord.Y);
	}

	TArray<FHhAxialCoord, TInlineAllocator<6>> GetNeighborLocations(const FHhAxialCoord& Location)
	{
		TArray<FHhAxialCoord, TInlineAllocator<6>> Result;
		for (const FHhAxialCoord& Direction : NeighborOffsets)
		{
			Result.Add(Offset(Location, Direction));
		}
		return Result;
	}

	namespace Private
	{
		using FCells = TMap<FHhAxialCoord, UInventoryItem*>;

		FCells MakeCells(const FLayout& Layout)
		{
			FCells Cells;
			Cells.Reserve(Layout.Num());
			for (const TPair<UInventoryItem*, FHhAxialCoord>& Placed : Layout)
			{
				Cells.Add(Placed.Value, Placed.Key);
			}
			return Cells;
		}

		/** Neighbours of Item in Layout (empty if Item is not placed there). */
		TArray<UInventoryItem*, TInlineAllocator<6>> NeighborsOf(UInventoryItem* Item, const FLayout& Layout, const FCells& Cells)
		{
			TArray<UInventoryItem*, TInlineAllocator<6>> Result;
			if (const FHhAxialCoord* Location = Layout.Find(Item))
			{
				for (const FHhAxialCoord& NeighborLocation : GetNeighborLocations(*Location))
				{
					if (UInventoryItem* const* Neighbor = Cells.Find(NeighborLocation))
					{
						Result.Add(*Neighbor);
					}
				}
			}
			return Result;
		}
	}

	FInventoryLayoutChange Diff(const FLayout& Before, const FLayout& After)
	{
		FInventoryLayoutChange Change;

		// Placements: removals first, then moves / additions in the order of After.
		for (const TPair<UInventoryItem*, FHhAxialCoord>& Old : Before)
		{
			if (!After.Contains(Old.Key))
			{
				FInventoryItemPlacementChange& Removed = Change.Placements.AddDefaulted_GetRef();
				Removed.Item = Old.Key;
				Removed.bWasPlaced = true;
				Removed.PrevLocation = Old.Value;
			}
		}
		for (const TPair<UInventoryItem*, FHhAxialCoord>& New : After)
		{
			const FHhAxialCoord* OldLocation = Before.Find(New.Key);
			if (OldLocation && *OldLocation == New.Value)
			{
				continue;
			}
			FInventoryItemPlacementChange& Placement = Change.Placements.AddDefaulted_GetRef();
			Placement.Item = New.Key;
			Placement.bWasPlaced = OldLocation != nullptr;
			Placement.PrevLocation = OldLocation ? *OldLocation : FHhAxialCoord();
			Placement.bIsPlaced = true;
			Placement.Location = New.Value;
		}

		if (Change.Placements.IsEmpty())
		{
			return Change;
		}

		// Neighbours: compare every item's neighbour set before / after (inventories are small).
		const Private::FCells CellsBefore = Private::MakeCells(Before);
		const Private::FCells CellsAfter = Private::MakeCells(After);

		auto Compare = [&](UInventoryItem* Item)
		{
			const auto Old = Private::NeighborsOf(Item, Before, CellsBefore);
			const auto New = Private::NeighborsOf(Item, After, CellsAfter);

			FInventoryItemNeighborsChange Neighbors;
			Neighbors.Item = Item;
			for (UInventoryItem* Neighbor : New)
			{
				if (!Old.Contains(Neighbor))
				{
					Neighbors.GainedNeighbors.Add(Neighbor);
				}
			}
			for (UInventoryItem* Neighbor : Old)
			{
				if (!New.Contains(Neighbor))
				{
					Neighbors.LostNeighbors.Add(Neighbor);
				}
			}
			if (!Neighbors.GainedNeighbors.IsEmpty() || !Neighbors.LostNeighbors.IsEmpty())
			{
				Change.Neighbors.Add(MoveTemp(Neighbors));
			}
		};

		for (const TPair<UInventoryItem*, FHhAxialCoord>& New : After)
		{
			Compare(New.Key);
		}
		for (const TPair<UInventoryItem*, FHhAxialCoord>& Old : Before)
		{
			if (!After.Contains(Old.Key))
			{
				Compare(Old.Key);
			}
		}
		return Change;
	}
}
