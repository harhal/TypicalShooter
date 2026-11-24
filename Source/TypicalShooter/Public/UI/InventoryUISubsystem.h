// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HhAxialCoord.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "UObject/ObjectKey.h"
#include "InventoryUISubsystem.generated.h"

class IItemsContainer_Model;
class UInventoryItem;

DECLARE_MULTICAST_DELEGATE(FOnInventorySelectionChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDisplayedInventoryItemChanged, UInventoryItem*, Item);

/**
 * Per local player inventory UI state shared by every grid:
 * - the hovered (mouse) and the selected (clicked / confirmed) item;
 * - the gamepad / keyboard cursor: a cell of one of the navigable containers (the open inventory screen's grids).
 * The item info panel shows the item under the active cursor, else the hovered item, else the selected one.
 */
UCLASS()
class TYPICALSHOOTER_API UInventoryUISubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	/** nullptr clears the hover. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetHoveredItem(UInventoryItem* Item);

	/** Selects Item; selecting the already selected item (or nullptr) clears the selection. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SelectItem(UInventoryItem* Item);

	/** Drops every reference to an item that left the UI (removed from its container). */
	void ForgetItem(const UInventoryItem* Item);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	UInventoryItem* GetHoveredItem() const { return HoveredItem.Get(); }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	UInventoryItem* GetSelectedItem() const { return SelectedItem.Get(); }

	/** Item under the active cursor if any, otherwise the hovered item, otherwise the selected one. */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	UInventoryItem* GetDisplayedItem() const;

	// --- Gamepad / keyboard cursor ---------------------------------------------------------------------------------

	/**
	 * Containers (objects implementing IItemsContainer_Model) the cursor can visit, in switching order. The cursor
	 * stays in its container if that one is still listed, otherwise it goes to the first one. Empty = hide the cursor.
	 */
	void SetNavigableContainers(const TArray<UObject*>& Containers);

	/** The container the cursor is in (nullptr if none is navigable). */
	UFUNCTION(BlueprintPure, Category = "Inventory|Cursor")
	UObject* GetCursorContainer() const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Cursor")
	FHhAxialCoord GetCursorLocation() const { return CursorLocation; }

	/** The cursor is drawn (and drives the info panel) only while active: from the first gamepad / key navigation until the mouse takes over. */
	UFUNCTION(BlueprintPure, Category = "Inventory|Cursor")
	bool IsCursorActive() const { return bCursorActive && GetCursorContainer() != nullptr; }

	void SetCursorActive(bool bActive);

	/**
	 * Moves the cursor to the adjacent cell best matching Direction (x = right, y = up). Straight up / down zig-zags
	 * between the two upper / lower neighbours so the cursor keeps its column. Returns false at the grid edge.
	 */
	bool MoveCursor(const FVector2D& Direction);

	/** Moves the cursor to the next (Step > 0) / previous navigable container, wrapping around; each remembers its cell. */
	void SwitchCursorContainer(int32 Step);

	UFUNCTION(BlueprintPure, Category = "Inventory|Cursor")
	UInventoryItem* GetItemAtCursor() const;

	/**
	 * Gamepad "A": with nothing selected selects the item under the cursor; on the selected item deselects it;
	 * otherwise asks to move the selected item to the cursor cell (swap if occupied, any navigable container).
	 */
	void ConfirmAtCursor();

	/** Clears the selection; false if nothing was selected. */
	bool CancelSelection();

	/** What a "take" command acts on: the item under the active cursor, otherwise the selected item. */
	UInventoryItem* GetFocusedItem() const;

	/** Hover, selection or cursor changed. */
	FOnInventorySelectionChanged OnSelectionChanged;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnDisplayedInventoryItemChanged OnDisplayedItemChanged;

private:
	void NotifyChanged();
	IItemsContainer_Model* GetCursorModel() const;
	/** Puts the cursor into NavigableContainers[Index] at the cell remembered for it (centre by default). */
	void EnterContainer(int32 Index);
	/** Screen-space x of a cell centre (hex size 1). */
	static double GetCellScreenX(const FHhAxialCoord& Location);

	TWeakObjectPtr<UInventoryItem> HoveredItem;
	TWeakObjectPtr<UInventoryItem> SelectedItem;

	TArray<TWeakObjectPtr<UObject>> NavigableContainers;
	int32 CursorContainerIndex = INDEX_NONE;
	FHhAxialCoord CursorLocation;
	/** Column the cursor tries to keep when moving vertically (screen x, hex size 1). */
	double CursorAnchorX = 0.0;
	TMap<TObjectKey<UObject>, FHhAxialCoord> RememberedCursorLocations;
	bool bCursorActive = false;
};
