// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

struct FHhAxialCoord;

class TYPICALSHOOTER_API IItemsContainer_View
{
public:
    virtual ~IItemsContainer_View() = default;
	
	virtual FGuid AddInventoryItem(class UInventoryItem_Presenter* Item, const FHhAxialCoord& Location) = 0;
	virtual void MoveInventoryItem(const FGuid& SlotGuid, const FHhAxialCoord& Location) = 0;
	virtual void RemoveInventoryItem(const FGuid& SlotGuid) = 0;
	
	virtual void GetAddItemDelegate() = 0;
	virtual void GetRemoveItemDelegate() = 0;
	virtual void GetMoveItemDelegate() = 0;
};
