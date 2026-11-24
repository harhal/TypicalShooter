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

/**
 * Keeps a container view in sync with a container model (initial fill + add/remove/move events).
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
};
