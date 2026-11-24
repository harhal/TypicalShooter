// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HhAxialCoord.h"
#include "InventoryItem.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "UObject/Object.h"
#include "InventoryArray.generated.h"

class UInventoryItem;
class UInventoryComponent;

/**
 * One item placed at a hex cell of an inventory.
*/
USTRUCT()
struct TYPICALSHOOTER_API FInventoryArray_Entry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FInventoryArray_Entry() {}

	FInventoryArray_Entry(UInventoryItem* InItem, const FHhAxialCoord& InLocation):
		Item(InItem),
		Location(InLocation),
		LastKnownLocation(InLocation)
	{}

private:
	friend struct FInventoryArray;
	friend class UInventoryComponent;

	UPROPERTY()
	TObjectPtr<UInventoryItem> Item = nullptr;

	UPROPERTY()
	FHhAxialCoord Location;

	/** Not replicated: location before the latest replicated change, used to report moves on clients. */
	UPROPERTY(NotReplicated)
	FHhAxialCoord LastKnownLocation;

	/** Not replicated: whether listeners were told about this entry (the item object may resolve after the entry arrives). */
	UPROPERTY(NotReplicated)
	bool bAddNotified = false;
};

/**
 * Replicated list of items placed on a hex grid. Mutations are server-only.
 * Inventories are small, so lookups by item / location are linear.
 */
USTRUCT()
struct TYPICALSHOOTER_API FInventoryArray : public FFastArraySerializer
{
	GENERATED_BODY()

	void SetOwner(UInventoryComponent* InOwner) { Owner = InOwner; }

	UInventoryItem* AddItem(const struct FInventoryItemManifest& ItemManifest, const FHhAxialCoord& Location);
	bool AddItem(UInventoryItem* Item, const FHhAxialCoord& Location);

	void RemoveItem(const UInventoryItem* Item);
	void RemoveItem(const FHhAxialCoord& Location);

	bool MoveItem(const UInventoryItem* Item, const FHhAxialCoord& Location);

	FORCEINLINE int32 GetNumItems() const { return Entries.Num(); }

	bool IsLocationOccupied(const FHhAxialCoord& Location) const { return FindIndexByLocation(Location) != INDEX_NONE; }

	TArray<TPair<UInventoryItem*, FHhAxialCoord>> GetPlacedItems() const;

	/** Registers all current items as replicated subobjects of the owner (called when the owner becomes ready for replication). */
	void RegisterReplicatedSubObjects();

	void PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo & DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FInventoryArray_Entry, FInventoryArray>( Entries, DeltaParms, *this );
	}

private:
	bool HasAuthority() const;
	int32 FindIndexByItem(const UInventoryItem* Item) const;
	int32 FindIndexByLocation(const FHhAxialCoord& Location) const;
	void RemoveItemAt(const int32 Idx);

	UPROPERTY()
	TArray<FInventoryArray_Entry> Entries;

	/** Set by the owning component in PostInitProperties; intentionally not a UPROPERTY so it is never copied from archetypes. */
	TWeakObjectPtr<UInventoryComponent>	Owner;
};

template<>
struct TStructOpsTypeTraits<FInventoryArray> : public TStructOpsTypeTraitsBase2<FInventoryArray>
{
	enum { WithNetDeltaSerializer = true };
};
