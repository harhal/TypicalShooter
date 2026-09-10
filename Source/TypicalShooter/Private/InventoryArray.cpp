// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryArray.h"

#include "InventoryComponent.h"
#include "InventoryItem.h"
#include "InventoryItemManifest.h"

void FInventoryArray::AddItem(const FInventoryItemManifest& ItemManifest, const FHhAxialCoord& Location)
{
	if (!Owner.IsValid() || !Owner->GetOwner()->HasAuthority())
	{
		return;
	}

	UInventoryItem* Item = ItemManifest.ManifestItem(Owner.Get());
	if (!ensureAlways(IsValid(Item)))
	{
		return;
	}
	
	AddItem(Item, Location);
}

void FInventoryArray::AddItem(UInventoryItem* Item, const FHhAxialCoord& Location)
{
	CoordsMap.Add(Location, Entries.Num());
	FInventoryArray_Entry& NewEntry = Entries.Add_GetRef(FInventoryArray_Entry(TStrongObjectPtr(Item), Location));
	
	if (Owner->IsUsingRegisteredSubObjectList() && Owner->IsReadyForReplication())
	{
		Owner->AddReplicatedSubObject(Item);
	}
	
	Owner->ProcessItemAdded(Item, Location);
	
	MarkItemDirty(NewEntry);
}

void FInventoryArray::RemoveItem(const UInventoryItem* Item)
{
	const int32 ItemIndex = Entries.IndexOfByPredicate([Item](const FInventoryArray_Entry& Entry)
	{
		return Entry.Item.Get() == Item;
	});
	
	RemoveItem(ItemIndex);
}

void FInventoryArray::RemoveItem(const FHhAxialCoord& Location)
{
	if (!CoordsMap.Contains(Location))
	{
		return;
	}

	const int32 ItemIndex = CoordsMap[Location];
	RemoveItem(ItemIndex);
}

TArray<TPair<UInventoryItem*, FHhAxialCoord>> FInventoryArray::GetPlacedItems() const
{
	TArray<TPair<UInventoryItem*, FHhAxialCoord>> Result;
	Algo::Transform(Entries, Result, [](const FInventoryArray_Entry& Entry)
	{
		return TPair<UInventoryItem*, FHhAxialCoord>(Entry.Item.Get(), Entry.Location);
	});
	return Result;
}

void FInventoryArray::RemoveItem(const int32 Idx)
{
	if (Idx == INDEX_NONE)
	{
		return;
	}
	
	Owner->ProcessItemRemoved(Entries[Idx].Item.Get(), Entries[Idx].Location);
	
	if (Owner->IsUsingRegisteredSubObjectList() && Owner->IsReadyForReplication())
	{
		Owner->RemoveReplicatedSubObject(Entries[Idx].Item.Get());
	}
	
	CoordsMap.Remove(Entries[Idx].Location);
	
	Entries.RemoveAtSwap(Idx);
	CoordsMap[Entries[Idx].Location] = Idx;
	
	MarkArrayDirty();
}

void FInventoryArray::MoveItem(const UInventoryItem* Item, const FHhAxialCoord& Location)
{
	const int32 ItemIndex = Entries.IndexOfByPredicate([Item](const FInventoryArray_Entry& Entry)
	{
		return Entry.Item.Get() == Item;
	});
	if (ItemIndex == INDEX_NONE)
	{
		return;
	}
	
	const FHhAxialCoord PrevLocation = Entries[ItemIndex].Location;
	
	CoordsMap.Remove(PrevLocation);
	CoordsMap[Location] = ItemIndex;
	
	Entries[ItemIndex].Location = Location;
	
	Owner->ProcessItemMoved(Entries[ItemIndex].Item.Get(), Entries[ItemIndex].Location, PrevLocation);
	
	MarkItemDirty(Entries[ItemIndex]);
}

void FInventoryArray::PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize)
{
	if (!Owner.IsValid())
	{
		return;
	}
	
	for (const int32 Idx : RemovedIndices)
	{
		CoordsMap.Remove(Entries[Idx].Location);
		Owner->ProcessItemRemoved(Entries[Idx].Item.Get(), Entries[Idx].Location);
	}
}

void FInventoryArray::PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize)
{
	if (!Owner.IsValid())
	{
		return;
	}
	
	for (const int32 Idx : AddedIndices)
	{
		CoordsMap.Add(Entries[Idx].Location, Idx);
		Owner->ProcessItemAdded(Entries[Idx].Item.Get(), Entries[Idx].Location);
	}
}

void FInventoryArray::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize)
{
	if (!Owner.IsValid())
	{
		return;
	}
	
	for (const int32 Idx : ChangedIndices)
	{
		const FHhAxialCoord* PrevCoords = CoordsMap.FindKey(Idx);
		if (PrevCoords)
		{
			CoordsMap.Remove(*PrevCoords);
		}
		CoordsMap.Add(Entries[Idx].Location, Idx);
		Owner->ProcessItemMoved(Entries[Idx].Item.Get(), Entries[Idx].Location, PrevCoords ? *PrevCoords : FHhAxialCoord());
	}
}
