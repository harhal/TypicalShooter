#include "InventoryComponent.h"

#include "InventoryItem.h"
#include "Net/UnrealNetwork.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;
}

void UInventoryComponent::AddItem(const struct FInventoryItemManifest& ItemManifest, const FHhAxialCoord& Location)
{
	InventoryArray.AddItem(ItemManifest, Location);
}

void UInventoryComponent::AddItem(UInventoryItem* Item, const FHhAxialCoord& Location)
{
	InventoryArray.AddItem(Item, Location);
}

void UInventoryComponent::RemoveItem(const UInventoryItem* Item)
{
	InventoryArray.RemoveItem(Item);
}

void UInventoryComponent::RemoveItem(const FHhAxialCoord& Location)
{
	InventoryArray.RemoveItem(Location);
}

void UInventoryComponent::MoveItem(const UInventoryItem* Item, const FHhAxialCoord& Location)
{
	InventoryArray.MoveItem(Item, Location);
}

void UInventoryComponent::ProcessItemAdded(const UInventoryItem* Item, const FHhAxialCoord& Location) const
{
	OnItemAddedDelegate.Broadcast(Item, Location);
}

void UInventoryComponent::ProcessItemRemoved(const UInventoryItem* Item, const FHhAxialCoord& Location) const
{
	OnItemRemovedDelegate.Broadcast(Item);
}

void UInventoryComponent::ProcessItemMoved(const UInventoryItem* Item, const FHhAxialCoord& Location, const FHhAxialCoord& PrevLocation) const
{
	OnItemMovedDelegate.Broadcast(Item, Location, PrevLocation);
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, InventoryArray);
}

TArray<TPair<UInventoryItem*, FHhAxialCoord>> UInventoryComponent::GetPlacedItems() const
{
	return InventoryArray.GetPlacedItems();
}

FItemAddedDelegate UInventoryComponent::GetItemAddedDelegate() const
{
	return OnItemAddedDelegate;
}

FItemRemovedDelegate UInventoryComponent::GetItemRemovedDelegate() const
{
	return OnItemRemovedDelegate;
}

FItemMovedDelegate UInventoryComponent::GetItemMovedDelegate() const
{
	return OnItemMovedDelegate;
}
