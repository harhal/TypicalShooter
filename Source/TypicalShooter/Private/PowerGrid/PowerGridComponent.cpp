#include "PowerGrid/PowerGridComponent.h"

#include "InventoryComponent.h"
#include "InventoryItem.h"
#include "PowerGrid/PowerGridFragment.h"

namespace PowerGridComponentPrivate
{
	const FPowerGrid_ItemFragment* GetPowerFragment(const UInventoryItem* Item)
	{
		const FInventoryItemManifest* Manifest = Item ? Item->GetItemManifest() : nullptr;
		return Manifest ? Manifest->GetFragmentOfType<FPowerGrid_ItemFragment>() : nullptr;
	}

	FPowerGridNode MakeNode(const UInventoryItem& Item, const FPowerGrid_ItemFragment& Fragment, const FHhAxialCoord& Location)
	{
		FPowerGridNode Node;
		Node.Location = Location;
		Node.Type = Fragment.ItemType;
		Node.OpenFaces = Fragment.GetOpenFaces(Item.GetRotation());
		Node.SourcePower = Fragment.Source.Power;
		// Clips are not implemented yet: a source needing ammo has an empty clip and does not work (R9d).
		Node.bSourceWorking = !Fragment.Source.NeedsAmmo();
		return Node;
	}
}

void UPowerGridComponent::OnRegister()
{
	Super::OnRegister();
	if (!Inventory.IsValid())
	{
		SetInventory(UInventoryComponent::FindInventory(GetOwner()));
	}
}

void UPowerGridComponent::OnUnregister()
{
	SetInventory(nullptr);
	Super::OnUnregister();
}

void UPowerGridComponent::SetInventory(UInventoryComponent* InInventory)
{
	if (UInventoryComponent* Old = Inventory.Get())
	{
		Old->GetLayoutChangedDelegate().Remove(LayoutChangedHandle);
	}
	LayoutChangedHandle.Reset();
	Inventory = InInventory;
	if (InInventory)
	{
		LayoutChangedHandle = InInventory->GetLayoutChangedDelegate().AddUObject(this, &ThisClass::HandleLayoutChanged);
	}
	Recalculate();
}

void UPowerGridComponent::Recalculate()
{
	using namespace PowerGridComponentPrivate;
	NodeItems.Reset();
	Nodes.Reset();
	if (const UInventoryComponent* InventoryPtr = Inventory.Get())
	{
		for (const TPair<UInventoryItem*, FHhAxialCoord>& Placed : InventoryPtr->GetPlacedItems())
		{
			if (const FPowerGrid_ItemFragment* Fragment = GetPowerFragment(Placed.Key))
			{
				Nodes.Add(MakeNode(*Placed.Key, *Fragment, Placed.Value));
				NodeItems.Add(Placed.Key);
			}
		}
	}
	Solution = PowerGridSolver::Solve(Nodes);
	PowerGridChangedDelegate.Broadcast(this);
}

void UPowerGridComponent::HandleLayoutChanged(UInventoryComponent* ChangedInventory, const FInventoryLayoutChange& Change)
{
	Recalculate();
}

int32 UPowerGridComponent::FindNode(const UInventoryItem* Item) const
{
	return Item ? NodeItems.IndexOfByPredicate([Item](const TWeakObjectPtr<UInventoryItem>& Node) { return Node.Get() == Item; }) : INDEX_NONE;
}

int32 UPowerGridComponent::GetItemPower(const UInventoryItem* Item) const
{
	const int32 Node = FindNode(Item);
	return Node != INDEX_NONE ? Solution.Nodes[Node].Power : 0;
}

bool UPowerGridComponent::IsItemPowered(const UInventoryItem* Item) const
{
	const int32 Node = FindNode(Item);
	return Node != INDEX_NONE && Solution.Nodes[Node].bPowered;
}

TArray<UInventoryItem*> UPowerGridComponent::GetConnectedItems(const UInventoryItem* Receiver) const
{
	TArray<UInventoryItem*> Items;
	const int32 Node = FindNode(Receiver);
	if (Node != INDEX_NONE)
	{
		for (const int32 Connected : Solution.Nodes[Node].Connected)
		{
			if (UInventoryItem* Item = NodeItems[Connected].Get())
			{
				Items.Add(Item);
			}
		}
	}
	return Items;
}

UInventoryItem* UPowerGridComponent::GetSlotReceiver(const EReceiverSlot Slot) const
{
	const FHhAxialCoord Cell = ReceiverSlots::GetLocation(Slot);
	const int32 Node = Nodes.IndexOfByPredicate([&Cell](const FPowerGridNode& Candidate)
	{
		return Candidate.Type == EPowerItemType::Receiver && Candidate.Location == Cell;
	});
	return Node != INDEX_NONE ? NodeItems[Node].Get() : nullptr;
}

UInventoryItem* UPowerGridComponent::GetActiveReceiver(const EReceiverSlot Slot) const
{
	UInventoryItem* Receiver = GetSlotReceiver(Slot);
	const FPowerGrid_ItemFragment* Fragment = PowerGridComponentPrivate::GetPowerFragment(Receiver);
	return Fragment && GetItemPower(Receiver) >= Fragment->Receiver.RequiredPower ? Receiver : nullptr;
}

TOptional<EReceiverSlot> UPowerGridComponent::GetItemSlot(const UInventoryItem* Item) const
{
	const int32 Node = FindNode(Item);
	return Node != INDEX_NONE && Nodes[Node].Type == EPowerItemType::Receiver
		? ReceiverSlots::GetSlotAt(Nodes[Node].Location) : TOptional<EReceiverSlot>();
}

TArray<UInventoryItem*> UPowerGridComponent::GetNodeItems() const
{
	TArray<UInventoryItem*> Items;
	for (const TWeakObjectPtr<UInventoryItem>& Item : NodeItems)
	{
		Items.Add(Item.Get());
	}
	return Items;
}
