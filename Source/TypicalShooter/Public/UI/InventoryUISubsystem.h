// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "InventoryUISubsystem.generated.h"

class UInventoryItem;

DECLARE_MULTICAST_DELEGATE(FOnInventorySelectionChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDisplayedInventoryItemChanged, UInventoryItem*, Item);

/**
 * Per local player inventory UI state shared by every grid: the hovered and the selected (clicked) item.
 * The item info panel shows the hovered item, falling back to the selected one.
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

	/** Hovered item if any, otherwise the selected one. */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	UInventoryItem* GetDisplayedItem() const;

	/** Hover or selection changed. */
	FOnInventorySelectionChanged OnSelectionChanged;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnDisplayedInventoryItemChanged OnDisplayedItemChanged;

private:
	void NotifyChanged();

	TWeakObjectPtr<UInventoryItem> HoveredItem;
	TWeakObjectPtr<UInventoryItem> SelectedItem;
};
