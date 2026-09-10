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
 * 
*/
USTRUCT()
struct TYPICALSHOOTER_API FInventoryArray_Entry : public FFastArraySerializerItem
{
	GENERATED_BODY()
	
	FInventoryArray_Entry() {}
	
	FInventoryArray_Entry(const TStrongObjectPtr<UInventoryItem>& InItem, const FHhAxialCoord& InLocation):
		Item(InItem),
		Location(InLocation)
	{}
	
private:	
	friend struct FInventoryArray;	
	friend class UInventoryComponent;
	
	TStrongObjectPtr<UInventoryItem> Item;
	FHhAxialCoord Location;
};

USTRUCT()
struct TYPICALSHOOTER_API FInventoryArray : public FFastArraySerializer
{
	GENERATED_BODY()
	
	void AddItem(const struct FInventoryItemManifest& ItemManifest, const FHhAxialCoord& Location);	
	void AddItem(UInventoryItem* Item, const FHhAxialCoord& Location);
	
	void RemoveItem(const UInventoryItem* Item);	
	void RemoveItem(const FHhAxialCoord& Location);
	
	void MoveItem(const UInventoryItem* Item, const FHhAxialCoord& Location);
	
	FORCEINLINE int32 GetNumItems() const { return Entries.Num(); }

	TArray<TPair<UInventoryItem*, FHhAxialCoord>> GetPlacedItems() const;
	
	void PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize);
	
	bool NetDeltaSerialize(FNetDeltaSerializeInfo & DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FInventoryArray_Entry, FInventoryArray>( Entries, DeltaParms, *this );
	}

private:
	void RemoveItem(const int32 Idx);	
	
	UPROPERTY()
	TArray<FInventoryArray_Entry> Entries;
	
	TMap<FHhAxialCoord, int32> CoordsMap;
	
	UPROPERTY(NotReplicated)
	TWeakObjectPtr<UInventoryComponent>	Owner;
};

template<>
struct TStructOpsTypeTraits<FInventoryArray> : public TStructOpsTypeTraitsBase2<FInventoryArray>
{
	enum { WithNetDeltaSerializer = true };
};
