// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItem_Model.h"
#include "UObject/Object.h"
#include "InventoryItem_Presenter.generated.h"

class IInventoryItem_View;
/**
 * 
 */
UCLASS(BlueprintType)
class TYPICALSHOOTER_API UInventoryItem_Presenter : public UObject
{
	GENERATED_BODY()
	
	UPROPERTY()
	TScriptInterface<IInventoryItem_Model> Model;
	
	TSharedPtr<IInventoryItem_View> View;

public:	
	UFUNCTION(BlueprintCallable)
	void SetModel(TScriptInterface<IInventoryItem_Model> InModel);	
	void SetView(TSharedPtr<IInventoryItem_View> InView);
	
private:
	void UpdateView() const;
};