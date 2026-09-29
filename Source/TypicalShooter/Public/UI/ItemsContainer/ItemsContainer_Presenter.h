// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HhAxialCoord.h"
#include "InventoryItem.h"
#include "UObject/Object.h"
#include "ItemsContainer_Presenter.generated.h"

class IItemsContainer_View;
class IItemsContainer_Model;

/**
 * 
 */
UCLASS(BlueprintType)
class TYPICALSHOOTER_API UItemsContainer_Presenter : public UObject
{
	GENERATED_BODY()
	
	UPROPERTY()
	TScriptInterface<IItemsContainer_Model> Model;
	
	TSharedPtr<IItemsContainer_View> View;
	
	void UpdateView() const;

public:
	UFUNCTION(BlueprintCallable)
	void SetModel(const TScriptInterface<IItemsContainer_Model>& InModel);	
	void SetView(const TSharedPtr<IItemsContainer_View>& InView);
	
private:
	void OnItemAdded(const UInventoryItem* InventoryItem, const FHhAxialCoord& HhAxialCoord) const;
	void OnItemRemoved(const UInventoryItem* InventoryItem) const;
	void OnItemMoved(const UInventoryItem* InventoryItem, const FHhAxialCoord& HhAxialCoord, const FHhAxialCoord& HhAxialCoord1) const;
};