#include "InventoryComponent.h"

#include "Engine/World.h"
#include "HhGridFunctions.h"
#include "InventoryItem.h"
#include "Net/UnrealNetwork.h"
#include "TypicalShooterPlayerController.h"

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

void UInventoryComponent::OnRegister()
{
	Super::OnRegister();

	// Safety net: the owner must always be this instance before any item is added.
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

bool UInventoryComponent::TransferItem(UInventoryItem* Item, UInventoryComponent* Target, const FHhAxialCoord& Location)
{
	const AActor* Owner = GetOwner();
	const AActor* TargetOwner = Target ? Target->GetOwner() : nullptr;
	if (!Item || !Owner || !Owner->HasAuthority() || !TargetOwner || !TargetOwner->HasAuthority())
	{
		return false;
	}

	const TOptional<FHhAxialCoord> SourceLocation = GetItemLocation(Item);
	if (!SourceLocation.IsSet() || !Target->IsLocationInBounds(Location))
	{
		return false;
	}

	UInventoryItem* Occupant = Target->GetItemAt(Location);

	if (Target == this)
	{
		if (!Occupant)
		{
			return MoveItem(Item, Location);
		}
		return Occupant == Item || InventoryArray.SwapItems(Item, Occupant);
	}

	// Between inventories items are re-created from their manifests: an item object is a replicated subobject
	// of exactly one component.
	const FInventoryItemManifest* ItemManifest = Item->GetItemManifest();
	const FInventoryItemManifest MovedManifest = ItemManifest ? *ItemManifest : FInventoryItemManifest();
	TOptional<FInventoryItemManifest> OccupantManifest;
	if (Occupant)
	{
		const FInventoryItemManifest* Manifest = Occupant->GetItemManifest();
		OccupantManifest = Manifest ? *Manifest : FInventoryItemManifest();
	}

	RemoveItem(Item);
	if (Occupant)
	{
		Target->RemoveItem(Occupant);
	}

	const bool bMoved = Target->AddItem(MovedManifest, Location) != nullptr;
	if (OccupantManifest.IsSet())
	{
		AddItem(OccupantManifest.GetValue(), SourceLocation.GetValue());
	}
	return ensure(bMoved);
}

bool UInventoryComponent::TransferItemToFreeLocation(UInventoryItem* Item, UInventoryComponent* Target)
{
	const TOptional<FHhAxialCoord> FreeLocation = Target ? Target->FindFreeLocation() : TOptional<FHhAxialCoord>();
	return FreeLocation.IsSet() && TransferItem(Item, Target, FreeLocation.GetValue());
}

void UInventoryComponent::SetGridRadius(const int32 InGridRadius)
{
	GridRadius = FMath::Max(0, InGridRadius);
}

bool UInventoryComponent::IsLocationInBounds(const FHhAxialCoord& Location) const
{
	return UHhGridFunctions::GetHexDistance(Location, FHhAxialCoord(0, 0)) <= GridRadius;
}

TOptional<FHhAxialCoord> UInventoryComponent::FindFreeLocation() const
{
	const int32 NumCells = UHhGridFunctions::GetHexSpiralLength(GridRadius);
	for (int32 N = 0; N < NumCells; ++N)
	{
		const FHhAxialCoord Location = UHhGridFunctions::MakeHexAxialCoordFromSpiral(N);
		if (!InventoryArray.IsLocationOccupied(Location))
		{
			return Location;
		}
	}
	return {};
}

UInventoryComponent* UInventoryComponent::FindInventory(UObject* ModelObject)
{
	if (UInventoryComponent* Inventory = Cast<UInventoryComponent>(ModelObject))
	{
		return Inventory;
	}
	const AActor* Actor = Cast<AActor>(ModelObject);
	return Actor ? Actor->FindComponentByClass<UInventoryComponent>() : nullptr;
}

void UInventoryComponent::RequestTransferItem(UInventoryItem* Item, const TScriptInterface<IItemsContainer_Model>& Target, const FHhAxialCoord& Location)
{
	UInventoryComponent* TargetInventory = FindInventory(Target.GetObject());
	if (!Item || !TargetInventory)
	{
		return;
	}

	// UI is client-local: the first player controller is the one driving it.
	const UWorld* World = GetWorld();
	if (ATypicalShooterPlayerController* PC = World ? Cast<ATypicalShooterPlayerController>(World->GetFirstPlayerController()) : nullptr)
	{
		PC->RequestTransferItem(this, Item, TargetInventory, Location);
		return;
	}

	// No controller (tools, tests): only possible with authority.
	TransferItem(Item, TargetInventory, Location);
}
