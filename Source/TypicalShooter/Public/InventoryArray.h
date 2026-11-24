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

	FInventoryArray() = default;

	/**
	 * Copies never take the owner: components of Blueprint actors get their properties copied from the class
	 * default object's component after PostInitProperties, which used to point the instance at the CDO's component.
	 */
	FInventoryArray(const FInventoryArray& Other)
		: FFastArraySerializer(Other)
		, Entries(Other.Entries)
	{
	}

	FInventoryArray& operator=(const FInventoryArray& Other)
	{
		if (this != &Other)
		{
			FFastArraySerializer::operator=(Other);
			Entries = Other.Entries;
		}
		return *this;
	}

	void SetOwner(UInventoryComponent* InOwner) { Owner = InOwner; }

	UInventoryItem* AddItem(const struct FInventoryItemManifest& ItemManifest, const FHhAxialCoord& Location);
	bool AddItem(UInventoryItem* Item, const FHhAxialCoord& Location);

	void RemoveItem(const UInventoryItem* Item);
	void RemoveItem(const FHhAxialCoord& Location);

	bool MoveItem(const UInventoryItem* Item, const FHhAxialCoord& Location);

	/** Exchanges the cells of two placed items. */
	bool SwapItems(const UInventoryItem* ItemA, const UInventoryItem* ItemB);

	UInventoryItem* GetItemAt(const FHhAxialCoord& Location) const;
	TOptional<FHhAxialCoord> GetItemLocation(const UInventoryItem* Item) const;

	FORCEINLINE int32 GetNumItems() const { return Entries.Num(); }

	bool IsLocationOccupied(const FHhAxialCoord& Location) const { return FindIndexByLocation(Location) != INDEX_NONE; }

	TArray<TPair<UInventoryItem*, FHhAxialCoord>> GetPlacedItems() const;

	/** Registers all current items as replicated subobjects of the owner (called when the owner becomes ready for replication). */
	void RegisterReplicatedSubObjects();

	void PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize);
	/** After every replication update (all removes / adds / changes applied): one consistent layout event. */
	void PostReplicatedReceive(const FFastArraySerializer::FPostReplicatedReceiveParameters& Parameters);

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

	/** Set by the owning component (PostInitProperties, OnRegister); not a UPROPERTY and skipped by the copy operations. */
	TWeakObjectPtr<UInventoryComponent>	Owner;
};

template<>
struct TStructOpsTypeTraits<FInventoryArray> : public TStructOpsTypeTraitsBase2<FInventoryArray>
{
	enum { WithNetDeltaSerializer = true };
};
