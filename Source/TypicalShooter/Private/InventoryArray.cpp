// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryArray.h"

#include "InventoryComponent.h"
#include "InventoryItem.h"
#include "InventoryItemManifest.h"

bool FInventoryArray::HasAuthority() const
{
	if (!Owner.IsValid())
	{
		return false;
	}

	const AActor* OwnerActor = Owner->GetOwner();
	return OwnerActor && OwnerActor->HasAuthority();
}

int32 FInventoryArray::FindIndexByItem(const UInventoryItem* Item) const
{
	return Entries.IndexOfByPredicate([Item](const FInventoryArray_Entry& Entry)
	{
		return Entry.Item == Item;
	});
}

int32 FInventoryArray::FindIndexByLocation(const FHhAxialCoord& Location) const
{
	return Entries.IndexOfByPredicate([&Location](const FInventoryArray_Entry& Entry)
	{
		return Entry.Location == Location;
	});
}

UInventoryItem* FInventoryArray::AddItem(const FInventoryItemManifest& ItemManifest, const FHhAxialCoord& Location)
{
	if (!HasAuthority() || IsLocationOccupied(Location))
	{
		return nullptr;
	}

	UInventoryItem* Item = ItemManifest.ManifestItem(Owner.Get());
	if (!ensureAlways(IsValid(Item)))
	{
		return nullptr;
	}

	return AddItem(Item, Location) ? Item : nullptr;
}

bool FInventoryArray::AddItem(UInventoryItem* Item, const FHhAxialCoord& Location)
{
	if (!HasAuthority() || !IsValid(Item))
	{
		return false;
	}

	if (IsLocationOccupied(Location) || FindIndexByItem(Item) != INDEX_NONE)
	{
		return false;
	}

	FInventoryArray_Entry& NewEntry = Entries.Add_GetRef(FInventoryArray_Entry(Item, Location));
	NewEntry.bAddNotified = true;

	if (Owner->IsUsingRegisteredSubObjectList() && Owner->IsReadyForReplication())
	{
		Owner->AddReplicatedSubObject(Item);
	}

	MarkItemDirty(NewEntry);

	Owner->ProcessItemAdded(Item, Location);

	return true;
}

void FInventoryArray::RemoveItem(const UInventoryItem* Item)
{
	RemoveItemAt(FindIndexByItem(Item));
}

void FInventoryArray::RemoveItem(const FHhAxialCoord& Location)
{
	RemoveItemAt(FindIndexByLocation(Location));
}

void FInventoryArray::RemoveItemAt(const int32 Idx)
{
	if (!HasAuthority() || !Entries.IsValidIndex(Idx))
	{
		return;
	}

	UInventoryItem* Item = Entries[Idx].Item;
	const FHhAxialCoord Location = Entries[Idx].Location;

	if (Item && Owner->IsUsingRegisteredSubObjectList())
	{
		Owner->RemoveReplicatedSubObject(Item);
	}

	Entries.RemoveAtSwap(Idx);
	MarkArrayDirty();

	Owner->ProcessItemRemoved(Item, Location);
}

bool FInventoryArray::MoveItem(const UInventoryItem* Item, const FHhAxialCoord& Location)
{
	if (!HasAuthority())
	{
		return false;
	}

	const int32 ItemIndex = FindIndexByItem(Item);
	if (ItemIndex == INDEX_NONE)
	{
		return false;
	}

	FInventoryArray_Entry& Entry = Entries[ItemIndex];
	const FHhAxialCoord PrevLocation = Entry.Location;
	if (PrevLocation == Location)
	{
		return true;
	}

	if (IsLocationOccupied(Location))
	{
		return false;
	}

	Entry.Location = Location;
	Entry.LastKnownLocation = Location;
	MarkItemDirty(Entry);

	Owner->ProcessItemMoved(Entry.Item, Location, PrevLocation);

	return true;
}

TArray<TPair<UInventoryItem*, FHhAxialCoord>> FInventoryArray::GetPlacedItems() const
{
	TArray<TPair<UInventoryItem*, FHhAxialCoord>> Result;
	Result.Reserve(Entries.Num());
	for (const FInventoryArray_Entry& Entry : Entries)
	{
		if (Entry.Item)
		{
			Result.Emplace(Entry.Item, Entry.Location);
		}
	}
	return Result;
}

void FInventoryArray::RegisterReplicatedSubObjects()
{
	if (!Owner.IsValid() || !Owner->IsUsingRegisteredSubObjectList())
	{
		return;
	}

	for (const FInventoryArray_Entry& Entry : Entries)
	{
		if (Entry.Item)
		{
			Owner->AddReplicatedSubObject(Entry.Item);
		}
	}
}

void FInventoryArray::PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize)
{
	if (!Owner.IsValid())
	{
		return;
	}

	for (const int32 Idx : RemovedIndices)
	{
		const FInventoryArray_Entry& Entry = Entries[Idx];
		if (Entry.bAddNotified)
		{
			Owner->ProcessItemRemoved(Entry.Item, Entry.LastKnownLocation);
		}
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
		FInventoryArray_Entry& Entry = Entries[Idx];
		Entry.LastKnownLocation = Entry.Location;
		if (Entry.Item)
		{
			Entry.bAddNotified = true;
			Owner->ProcessItemAdded(Entry.Item, Entry.Location);
		}
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
		FInventoryArray_Entry& Entry = Entries[Idx];
		const FHhAxialCoord PrevLocation = Entry.LastKnownLocation;
		Entry.LastKnownLocation = Entry.Location;

		if (!Entry.bAddNotified)
		{
			// Item object resolved after the entry itself arrived.
			if (Entry.Item)
			{
				Entry.bAddNotified = true;
				Owner->ProcessItemAdded(Entry.Item, Entry.Location);
			}
			continue;
		}

		if (PrevLocation != Entry.Location)
		{
			Owner->ProcessItemMoved(Entry.Item, Entry.Location, PrevLocation);
		}
	}
}
