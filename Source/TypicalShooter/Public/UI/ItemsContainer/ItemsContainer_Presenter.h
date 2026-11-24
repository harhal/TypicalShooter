// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HhAxialCoord.h"
#include "UI/ItemsContainer/ItemsContainer_Model.h"
#include "UObject/Object.h"
#include "UObject/ObjectKey.h"
#include "ItemsContainer_Presenter.generated.h"

class IItemsContainer_View;
class UInventoryItem;
class UInventoryItem_Presenter;

/** An item of the container the presenter shows (nullptr = none, e.g. the cursor left every item). */
DECLARE_MULTICAST_DELEGATE_OneParam(FContainerItemEvent, UInventoryItem*);

/**
 * Keeps a container view in sync with a container model (initial fill + add/remove/move events)
 * and turns view input (hover, click, drop) into model requests and UI events.
 */
UCLASS(BlueprintType)
class TYPICALSHOOTER_API UItemsContainer_Presenter : public UObject
{
	GENERATED_BODY()

public:
	/** Creates a presenter bound to InModel. The caller must keep a reference (e.g. UInventoryGridWidget::AssignPresenter). */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static UItemsContainer_Presenter* CreatePresenter(const TScriptInterface<IItemsContainer_Model>& InModel);

	virtual void BeginDestroy() override;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetModel(const TScriptInterface<IItemsContainer_Model>& InModel);
	void SetView(const TSharedPtr<IItemsContainer_View>& InView);

	const TScriptInterface<IItemsContainer_Model>& GetModel() const { return Model; }

	UInventoryItem* GetItemInSlot(const FGuid& SlotGuid) const;
	FGuid GetSlotOfItem(const UInventoryItem* Item) const;

	// View input
	/** An item of SourcePresenter's view was dropped at Location of this presenter's view. Returns true if a transfer was requested. */
	bool HandleItemDropped(UItemsContainer_Presenter* SourcePresenter, const FGuid& SourceSlot, const FHhAxialCoord& Location);
	/** The cursor moved onto the item in SlotGuid (invalid guid = onto no item). */
	void HandleItemHovered(const FGuid& SlotGuid);
	/** The item in SlotGuid was clicked (invalid guid = an empty cell). */
	void HandleItemClicked(const FGuid& SlotGuid);

	FContainerItemEvent OnItemHovered;
	FContainerItemEvent OnItemClicked;
	/** An item view was removed because the item left the container. */
	FContainerItemEvent OnItemViewRemoved;

private:
	void UnbindModel();
	void RebuildView();
	void AddItemToView(const UInventoryItem* Item, const FHhAxialCoord& Location);

	void OnItemAdded(const UInventoryItem* Item, const FHhAxialCoord& Location);
	void OnItemRemoved(const UInventoryItem* Item);
	void OnItemMoved(const UInventoryItem* Item, const FHhAxialCoord& Location, const FHhAxialCoord& PrevLocation);

	UPROPERTY()
	TScriptInterface<IItemsContainer_Model> Model;

	TWeakPtr<IItemsContainer_View> View;

	/** Item presenters by view slot; UPROPERTY keeps them alive. */
	UPROPERTY()
	TMap<FGuid, TObjectPtr<UInventoryItem_Presenter>> PresentersBySlot;

	TMap<TObjectKey<UInventoryItem>, FGuid> SlotsByItem;
	TMap<FGuid, TWeakObjectPtr<UInventoryItem>> ItemsBySlot;
};
