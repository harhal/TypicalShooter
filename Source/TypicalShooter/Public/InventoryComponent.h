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

	virtual void PostInitProperties() override;
	virtual void ReadyForReplication() override;

	/** Server only. Returns the created item or nullptr if the location is occupied. */
	UInventoryItem* AddItem(const FInventoryItemManifest& ItemManifest, const FHhAxialCoord& Location);
	bool AddItem(UInventoryItem* Item, const FHhAxialCoord& Location);
	void RemoveItem(const UInventoryItem* Item);
	void RemoveItem(const FHhAxialCoord& Location);
	bool MoveItem(const UInventoryItem* Item, const FHhAxialCoord& Location);
	FORCEINLINE int32 GetNumItems() const { return InventoryArray.GetNumItems(); }

	void ProcessItemAdded(const UInventoryItem* Item, const FHhAxialCoord& Location);
	void ProcessItemRemoved(const UInventoryItem* Item, const FHhAxialCoord& Location);
	void ProcessItemMoved(const UInventoryItem* Item, const FHhAxialCoord& Location, const FHhAxialCoord& PrevLocation);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// IItemsContainer_Model
	virtual TArray<TPair<UInventoryItem*, FHhAxialCoord>> GetPlacedItems() const override;
	virtual FItemAddedDelegate& GetItemAddedDelegate() override { return OnItemAddedDelegate; }
	virtual FItemRemovedDelegate& GetItemRemovedDelegate() override { return OnItemRemovedDelegate; }
	virtual FItemMovedDelegate& GetItemMovedDelegate() override { return OnItemMovedDelegate; }

private:
	UPROPERTY(Replicated)
	FInventoryArray InventoryArray;

	FItemAddedDelegate OnItemAddedDelegate;
	FItemRemovedDelegate OnItemRemovedDelegate;
	FItemMovedDelegate OnItemMovedDelegate;
};
