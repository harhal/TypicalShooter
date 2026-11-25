// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HhAxialCoord.h"
#include "InventoryLayout.generated.h"

class UInventoryItem;

/** Where one item was and is now. Added: !bWasPlaced; removed: !bIsPlaced; moved: both set, locations differ. */
USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FInventoryItemPlacementChange
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UInventoryItem> Item = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	bool bWasPlaced = false;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	FHhAxialCoord PrevLocation;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	bool bIsPlaced = false;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	FHhAxialCoord Location;

	bool IsAdded() const { return !bWasPlaced && bIsPlaced; }
	bool IsRemoved() const { return bWasPlaced && !bIsPlaced; }
	bool IsMoved() const { return bWasPlaced && bIsPlaced; }
};

/**
 * Items that became / stopped being adjacent (hex distance 1) to Item. An added item gains all its neighbours,
 * a removed item loses all of them; every pair is reported from both sides.
 */
USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FInventoryItemNeighborsChange
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UInventoryItem> Item = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TArray<TObjectPtr<UInventoryItem>> GainedNeighbors;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TArray<TObjectPtr<UInventoryItem>> LostNeighbors;
};

/** An item that stayed in the inventory but was rotated (clockwise 60-degree steps, 0..5). */
USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FInventoryItemRotationChange
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UInventoryItem> Item = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 PrevRotation = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 Rotation = 0;
};

/**
 * Everything one atomic inventory operation (add, remove, move, swap, transfer, take all...) changed, reported once
 * the inventory is consistent again: listeners never see a half-done swap or transfer.
 */
USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FInventoryLayoutChange
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TArray<FInventoryItemPlacementChange> Placements;

	/** Only items whose set of neighbours actually changed. */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TArray<FInventoryItemNeighborsChange> Neighbors;

	/** Items placed before and after whose rotation changed (added / removed items carry their own rotation). */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TArray<FInventoryItemRotationChange> Rotations;

	bool IsEmpty() const { return Placements.IsEmpty() && Neighbors.IsEmpty() && Rotations.IsEmpty(); }

	const FInventoryItemPlacementChange* FindPlacement(const UInventoryItem* Item) const;
	const FInventoryItemNeighborsChange* FindNeighbors(const UInventoryItem* Item) const;
	const FInventoryItemRotationChange* FindRotation(const UInventoryItem* Item) const;
};

/** Hex-grid layout helpers for inventories (pointy-top axial coordinates, as in HarhalHexagonalGrid). */
namespace InventoryLayout
{
	using FLayout = TMap<UInventoryItem*, FHhAxialCoord>;

	/** The six axial direction offsets, counter-clockwise starting from "right". */
	TYPICALSHOOTER_API TConstArrayView<FHhAxialCoord> GetNeighborOffsets();

	TYPICALSHOOTER_API FHhAxialCoord Offset(const FHhAxialCoord& Location, const FHhAxialCoord& Direction);

	/** The six cells adjacent to Location (not clipped to any grid). */
	TYPICALSHOOTER_API TArray<FHhAxialCoord, TInlineAllocator<6>> GetNeighborLocations(const FHhAxialCoord& Location);

	/** What turns Before into After: placement changes and per-item neighbour changes. */
	TYPICALSHOOTER_API FInventoryLayoutChange Diff(const FLayout& Before, const FLayout& After);

	using FRotations = TMap<UInventoryItem*, int32>;

	/** Items present in both maps whose rotation differs. */
	TYPICALSHOOTER_API TArray<FInventoryItemRotationChange> DiffRotations(const FRotations& Before, const FRotations& After);
}
