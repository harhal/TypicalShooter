// The power grid of an actor's inventory: solved from the placed items whenever the inventory layout changes
// (power grid R6b, R13a). Computed locally on the server and on clients from the replicated layout.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PowerGrid/PowerGridSolver.h"
#include "PowerGridComponent.generated.h"

class UInventoryComponent;
class UInventoryItem;
struct FInventoryLayoutChange;
class UPowerGridComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FPowerGridChangedDelegate, UPowerGridComponent* /*Grid*/);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TYPICALSHOOTER_API UPowerGridComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Follows InInventory's layout events (null = none). Done automatically for the owner's first inventory. */
	void SetInventory(UInventoryComponent* InInventory);
	UInventoryComponent* GetInventory() const { return Inventory.Get(); }

	/** Rebuilds the nodes from the inventory and solves the grid; broadcasts the change. */
	void Recalculate();

	bool IsInGrid(const UInventoryItem* Item) const { return FindNode(Item) != INDEX_NONE; }
	/** Incoming power (a source: its own power if it works); 0 outside the grid. */
	int32 GetItemPower(const UInventoryItem* Item) const;
	bool IsItemPowered(const UInventoryItem* Item) const;
	/** Receiver + items on powered paths to it (R14); empty for non-receivers. */
	TArray<UInventoryItem*> GetConnectedItems(const UInventoryItem* Receiver) const;

	/** Items of the grid nodes, parallel to GetSolution().Nodes. */
	TArray<UInventoryItem*> GetNodeItems() const;
	const FPowerGridSolution& GetSolution() const { return Solution; }

	/** Fires after every recalculation. */
	FPowerGridChangedDelegate& GetPowerGridChangedDelegate() { return PowerGridChangedDelegate; }

protected:
	virtual void OnRegister() override;
	virtual void OnUnregister() override;

private:
	int32 FindNode(const UInventoryItem* Item) const;
	void HandleLayoutChanged(UInventoryComponent* ChangedInventory, const FInventoryLayoutChange& Change);

	TWeakObjectPtr<UInventoryComponent> Inventory;
	FDelegateHandle LayoutChangedHandle;

	TArray<TWeakObjectPtr<UInventoryItem>> NodeItems;
	FPowerGridSolution Solution;
	FPowerGridChangedDelegate PowerGridChangedDelegate;
};
