// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

struct FHhAxialCoord;
class UInventoryItem_Presenter;

class TYPICALSHOOTER_API IItemsContainer_View
{
public:
	virtual ~IItemsContainer_View() = default;

	/** Creates a view for the item at Location, binds it to the presenter, returns a handle for later updates. */
	virtual FGuid AddInventoryItem(UInventoryItem_Presenter* Item, const FHhAxialCoord& Location) = 0;
	virtual void MoveInventoryItem(const FGuid& SlotGuid, const FHhAxialCoord& Location) = 0;
	virtual void RemoveInventoryItem(const FGuid& SlotGuid) = 0;
	virtual void ClearInventoryItems() = 0;
};
