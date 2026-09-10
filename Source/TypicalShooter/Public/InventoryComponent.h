// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryArray.h"
#include "Components/ActorComponent.h"
#include "UI/ItemsContainer/ItemsContainer_Model.h"
#include "InventoryComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TYPICALSHOOTER_API UInventoryComponent : public UActorComponent, public IItemsContainer_Model
{
	GENERATED_BODY()

public:
	UInventoryComponent();
	
	void AddItem(const FInventoryItemManifest& ItemManifest, const FHhAxialCoord& Location);	
	void AddItem(UInventoryItem* Item, const FHhAxialCoord& Location);	
	void RemoveItem(const UInventoryItem* Item);	
	void RemoveItem(const FHhAxialCoord& Location);	
	void MoveItem(const UInventoryItem* Item, const FHhAxialCoord& Location);	
	FORCEINLINE int32 GetNumItems() const { return InventoryArray.GetNumItems(); }

	void ProcessItemAdded(const UInventoryItem* Item, const FHhAxialCoord& Location) const;
	void ProcessItemRemoved(const UInventoryItem* Item, const FHhAxialCoord& Location) const;
	void ProcessItemMoved(const UInventoryItem* Item, const FHhAxialCoord& Location, const FHhAxialCoord& PrevLocation) const;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual TArray<TPair<UInventoryItem*, FHhAxialCoord>> GetPlacedItems() const override;
	virtual FItemAddedDelegate GetItemAddedDelegate() const override;
	virtual FItemRemovedDelegate GetItemRemovedDelegate() const override;
	virtual FItemMovedDelegate GetItemMovedDelegate() const override;

	UPROPERTY(Replicated)
	FInventoryArray InventoryArray;
	
	FItemAddedDelegate OnItemAddedDelegate;
	FItemRemovedDelegate OnItemRemovedDelegate;
	FItemMovedDelegate OnItemMovedDelegate;
};
