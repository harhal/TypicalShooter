// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItem_Model.h"
#include "UObject/Object.h"
#include "InventoryItem_Presenter.generated.h"

class IInventoryItem_View;
class UPowerGridComponent;

/**
 * Pushes item model data into an item view. For inventory items it also follows the power grid of the item's
 * inventory and refreshes the view's power info on every grid change (power grid R16).
 */
UCLASS(BlueprintType)
class TYPICALSHOOTER_API UInventoryItem_Presenter : public UObject
{
	GENERATED_BODY()

	UPROPERTY()
	TScriptInterface<IInventoryItem_Model> Model;

	TWeakPtr<IInventoryItem_View> View;

	TWeakObjectPtr<UPowerGridComponent> PowerGrid;
	FDelegateHandle PowerGridChangedHandle;

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetModel(const TScriptInterface<IInventoryItem_Model>& InModel);
	void SetView(const TSharedPtr<IInventoryItem_View>& InView);

	void UpdateView() const;

	virtual void BeginDestroy() override;

private:
	void BindPowerGrid();
	void UnbindPowerGrid();
};
