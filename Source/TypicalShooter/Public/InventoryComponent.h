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
	virtual void OnRegister() override;
	virtual void ReadyForReplication() override;

	/** Server only. Returns the created item or nullptr if the location is occupied. */
	UInventoryItem* AddItem(const FInventoryItemManifest& ItemManifest, const FHhAxialCoord& Location);
	bool AddItem(UInventoryItem* Item, const FHhAxialCoord& Location);
	void RemoveItem(const UInventoryItem* Item);
	void RemoveItem(const FHhAxialCoord& Location);
	bool MoveItem(const UInventoryItem* Item, const FHhAxialCoord& Location);
	FORCEINLINE int32 GetNumItems() const { return InventoryArray.GetNumItems(); }

	/**
	 * Server only. Puts Item (placed in this inventory) at Location of Target, which may be this inventory.
	 * If the target cell is occupied the two items swap places. Between inventories the item is re-created
	 * from its manifest in the target (items carry no state besides the manifest yet).
	 */
	bool TransferItem(UInventoryItem* Item, UInventoryComponent* Target, const FHhAxialCoord& Location);

	/** Server only. TransferItem to Target's first free cell; false if Target is full. */
	bool TransferItemToFreeLocation(UInventoryItem* Item, UInventoryComponent* Target);

	void SetGridRadius(int32 InGridRadius);
	bool IsLocationInBounds(const FHhAxialCoord& Location) const;

	/** First free cell in spiral order from the centre, unset if the grid is full. */
	TOptional<FHhAxialCoord> FindFreeLocation() const;

	UInventoryItem* GetItemAt(const FHhAxialCoord& Location) const { return InventoryArray.GetItemAt(Location); }
	TOptional<FHhAxialCoord> GetItemLocation(const UInventoryItem* Item) const { return InventoryArray.GetItemLocation(Item); }

	/** The inventory behind a container model object: the component itself or the first one on an actor. */
	static UInventoryComponent* FindInventory(UObject* ModelObject);

	void ProcessItemAdded(const UInventoryItem* Item, const FHhAxialCoord& Location);
	void ProcessItemRemoved(const UInventoryItem* Item, const FHhAxialCoord& Location);
	void ProcessItemMoved(const UInventoryItem* Item, const FHhAxialCoord& Location, const FHhAxialCoord& PrevLocation);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// IItemsContainer_Model
	virtual TArray<TPair<UInventoryItem*, FHhAxialCoord>> GetPlacedItems() const override;
	virtual FItemAddedDelegate& GetItemAddedDelegate() override { return OnItemAddedDelegate; }
	virtual FItemRemovedDelegate& GetItemRemovedDelegate() override { return OnItemRemovedDelegate; }
	virtual FItemMovedDelegate& GetItemMovedDelegate() override { return OnItemMovedDelegate; }
	virtual int32 GetGridRadius() const override { return GridRadius; }
	/** Routed through the local ATypicalShooterPlayerController's server RPC; applied directly with authority and no controller. */
	virtual void RequestTransferItem(UInventoryItem* Item, const TScriptInterface<IItemsContainer_Model>& Target, const FHhAxialCoord& Location) override;

private:
	/** Size of the hex grid: cells within this distance of the centre. Not replicated: set on the class / placed instance. */
	UPROPERTY(EditAnywhere, Category = "Inventory", meta = (ClampMin = 0, ClampMax = 8))
	int32 GridRadius = 2;

	UPROPERTY(Replicated)
	FInventoryArray InventoryArray;

	FItemAddedDelegate OnItemAddedDelegate;
	FItemRemovedDelegate OnItemRemovedDelegate;
	FItemMovedDelegate OnItemMovedDelegate;
};
