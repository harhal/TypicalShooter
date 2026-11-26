// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"
#include "UI/InventoryItem/ItemRarityDisplay.h"
#include "UObject/Interface.h"
#include "InventoryItem_Model.generated.h"

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UInventoryItem_Model : public UInterface
{
	GENERATED_BODY()
};

/** Data an inventory item exposes to UI. Names are prefixed with "Item" to avoid hiding UObject::GetName. */
class IInventoryItem_Model
{
	GENERATED_BODY()
public:
	virtual FSlateBrush GetItemIcon() const = 0;
	virtual FText GetItemName() const = 0;
	virtual FText GetItemDescription() const = 0;
	virtual int32 GetItemCount() const = 0;
	/** Unset for items without rarity (hand-made ones). */
	virtual TOptional<FItemRarityDisplay> GetItemRarity() const = 0;
};
