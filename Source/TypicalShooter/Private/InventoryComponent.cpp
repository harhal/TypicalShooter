#include "InventoryComponent.h"

#include "InventoryItem.h"
#include "Net/UnrealNetwork.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;
}

void UInventoryComponent::PostInitProperties()
{
	Super::PostInitProperties();

	// After property initialization so the owner is never left pointing at an archetype.
	InventoryArray.SetOwner(this);
}

void UInventoryComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	// Items added before the component was ready were not registered yet.
	InventoryArray.RegisterReplicatedSubObjects();
}

UInventoryItem* UInventoryComponent::AddItem(const FInventoryItemManifest& ItemManifest, const FHhAxialCoord& Location)
{
	return InventoryArray.AddItem(ItemManifest, Location);
}

bool UInventoryComponent::AddItem(UInventoryItem* Item, const FHhAxialCoord& Location)
{
	return InventoryArray.AddItem(Item, Location);
}

void UInventoryComponent::RemoveItem(const UInventoryItem* Item)
{
	InventoryArray.RemoveItem(Item);
}

void UInventoryComponent::RemoveItem(const FHhAxialCoord& Location)
{
	InventoryArray.RemoveItem(Location);
}

bool UInventoryComponent::MoveItem(const UInventoryItem* Item, const FHhAxialCoord& Location)
{
	return InventoryArray.MoveItem(Item, Location);
}

void UInventoryComponent::ProcessItemAdded(const UInventoryItem* Item, const FHhAxialCoord& Location)
{
	OnItemAddedDelegate.Broadcast(Item, Location);
}

void UInventoryComponent::ProcessItemRemoved(const UInventoryItem* Item, const FHhAxialCoord& Location)
{
	OnItemRemovedDelegate.Broadcast(Item);
}

void UInventoryComponent::ProcessItemMoved(const UInventoryItem* Item, const FHhAxialCoord& Location, const FHhAxialCoord& PrevLocation)
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
