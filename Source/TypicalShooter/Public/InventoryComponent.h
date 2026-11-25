// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryArray.h"
#include "InventoryLayout.h"
#include "Components/ActorComponent.h"
#include "UI/ItemsContainer/ItemsContainer_Model.h"
#include "InventoryComponent.generated.h"

class UInventoryComponent;

DECLARE_MULTICAST_DELEGATE_TwoParams(FInventoryLayoutChangedDelegate, UInventoryComponent* /*Inventory*/, const FInventoryLayoutChange& /*Change*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryLayoutChanged, UInventoryComponent*, Inventory, const FInventoryLayoutChange&, Change);

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
	 * in the target with UInventoryItem::CloneFor (manifest + instance state).
	 */
	bool TransferItem(UInventoryItem* Item, UInventoryComponent* Target, const FHhAxialCoord& Location);

	/** Server only. Rotates Item (placed in this inventory) clockwise by Steps * 60 degrees (power grid R3). */
	bool RotateItem(UInventoryItem* Item, int32 Steps = 1);

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

	/**
	 * Groups mutations into one layout event: while any batch of this inventory is alive nothing is reported; the
	 * outermost batch reports everything that changed when it ends. Every mutation opens its own batch, so a swap or
	 * a transfer is always one event; open one explicitly for multi-step operations (e.g. Take All).
	 */
	struct TYPICALSHOOTER_API FScopedLayoutBatch
	{
		explicit FScopedLayoutBatch(UInventoryComponent* InInventory);
		~FScopedLayoutBatch();
		UE_NONCOPYABLE(FScopedLayoutBatch);
	private:
		TWeakObjectPtr<UInventoryComponent> Inventory;
	};

	/**
	 * Fires once per atomic change of the layout (on the server after each mutation / batch, on clients after each
	 * replication update), when the inventory is consistent again. Carries placement and neighbour changes.
	 */
	FInventoryLayoutChangedDelegate& GetLayoutChangedDelegate() { return LayoutChangedDelegate; }

	/** Blueprint twin of GetLayoutChangedDelegate(). */
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryLayoutChanged OnLayoutChanged;

	/** Items in the (up to six) cells adjacent to Location. */
	TArray<UInventoryItem*> GetNeighborItems(const FHhAxialCoord& Location) const;

	/** Items adjacent to Item; empty if Item is not placed here. */
	TArray<UInventoryItem*> GetItemNeighbors(const UInventoryItem* Item) const;

	/** Reports layout changes since the last report, unless a batch is open (also called after replication updates). */
	void FlushLayoutChange();

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
	FInventoryLayoutChangedDelegate LayoutChangedDelegate;

	/** Open FScopedLayoutBatch count. */
	int32 LayoutBatchDepth = 0;

	struct FReportedPlacement
	{
		FHhAxialCoord Location;
		int32 Rotation = 0;
	};

	/** Layout as last reported to layout listeners. */
	TMap<TWeakObjectPtr<UInventoryItem>, FReportedPlacement> ReportedLayout;
};
