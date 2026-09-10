// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HhAxialCoord.h"
#include "UObject/Interface.h"
#include "ItemsContainer_Model.generated.h"

class UInventoryItem;

DECLARE_MULTICAST_DELEGATE_TwoParams(FItemAddedDelegate, const UInventoryItem*, const FHhAxialCoord&);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FItemMovedDelegate, const UInventoryItem*, const FHhAxialCoord&, const FHhAxialCoord&);
DECLARE_MULTICAST_DELEGATE_OneParam(FItemRemovedDelegate, const UInventoryItem*);

UINTERFACE(MinimalAPI)
class UItemsContainer_Model : public UInterface
{
	GENERATED_BODY()
};

class IItemsContainer_Model
{
	GENERATED_BODY()
public:
	virtual TArray<TPair<UInventoryItem*, FHhAxialCoord>> GetPlacedItems() const { return TArray<TPair<UInventoryItem*, FHhAxialCoord>>();}
	virtual FItemAddedDelegate GetItemAddedDelegate() const { return FItemAddedDelegate();}
	virtual FItemRemovedDelegate GetItemRemovedDelegate() const { return FItemRemovedDelegate(); }
	virtual FItemMovedDelegate GetItemMovedDelegate() const { return FItemMovedDelegate(); }
};
