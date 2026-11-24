// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItem_Model.h"
#include "UObject/Object.h"
#include "InventoryItem_Presenter.generated.h"

class IInventoryItem_View;
/**
 * Pushes item model data into an item view.
 */
UCLASS(BlueprintType)
class TYPICALSHOOTER_API UInventoryItem_Presenter : public UObject
{
	GENERATED_BODY()

	UPROPERTY()
	TScriptInterface<IInventoryItem_Model> Model;

	TWeakPtr<IInventoryItem_View> View;

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetModel(const TScriptInterface<IInventoryItem_Model>& InModel);
	void SetView(const TSharedPtr<IInventoryItem_View>& InView);

	void UpdateView() const;
};
