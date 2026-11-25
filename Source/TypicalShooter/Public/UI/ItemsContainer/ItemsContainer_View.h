// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

struct FHhAxialCoord;
struct FPowerGridDisplay;
class UInventoryItem_Presenter;
class UItemsContainer_Presenter;

class TYPICALSHOOTER_API IItemsContainer_View
{
public:
	virtual ~IItemsContainer_View() = default;

	/** The presenter user input (hover, click, drop) is reported to. */
	virtual void SetPresenter(UItemsContainer_Presenter* Presenter) = 0;

	/** Shows every cell within Radius of the centre. */
	virtual void SetGridRadius(int32 Radius) = 0;

	/** Creates a view for the item at Location, binds it to the presenter, returns a handle for later updates. */
	virtual FGuid AddInventoryItem(UInventoryItem_Presenter* Item, const FHhAxialCoord& Location) = 0;
	virtual void MoveInventoryItem(const FGuid& SlotGuid, const FHhAxialCoord& Location) = 0;
	virtual void RemoveInventoryItem(const FGuid& SlotGuid) = 0;
	virtual void ClearInventoryItems() = 0;

	/** Power grid of the container to draw over the items (empty = none). */
	virtual void SetPowerGrid(const FPowerGridDisplay& Display) = 0;
};
