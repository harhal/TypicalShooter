// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HhAxialCoord.h"
#include "UObject/Interface.h"
#include "ItemsContainer_Model.generated.h"

class UInventoryItem;

DECLARE_MULTICAST_DELEGATE_TwoParams(FItemAddedDelegate, const UInventoryItem*, const FHhAxialCoord&);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FItemMovedDelegate, const UInventoryItem*, const FHhAxialCoord& /*Location*/, const FHhAxialCoord& /*PrevLocation*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FItemRemovedDelegate, const UInventoryItem*);

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UItemsContainer_Model : public UInterface
{
	GENERATED_BODY()
};

class IItemsContainer_Model
{
	GENERATED_BODY()
public:
	virtual TArray<TPair<UInventoryItem*, FHhAxialCoord>> GetPlacedItems() const = 0;

	/** Delegates are returned by reference so listeners bind to the model's own instances. */
	virtual FItemAddedDelegate& GetItemAddedDelegate() = 0;
	virtual FItemRemovedDelegate& GetItemRemovedDelegate() = 0;
	virtual FItemMovedDelegate& GetItemMovedDelegate() = 0;

	/** Cells within this hex distance of (0,0) belong to the grid. */
	virtual int32 GetGridRadius() const = 0;

	/**
	 * Asks to put Item (placed in this container) at Location of Target (may be this container). An occupied target
	 * cell swaps the two items. Applied by the server; the result arrives through the delegates above.
	 */
	virtual void RequestTransferItem(UInventoryItem* Item, const TScriptInterface<IItemsContainer_Model>& Target, const FHhAxialCoord& Location) = 0;
};
