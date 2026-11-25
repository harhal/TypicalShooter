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
	FScopedLayoutBatch Batch(this);
	return InventoryArray.AddItem(ItemManifest, Location);
}

bool UInventoryComponent::AddItem(UInventoryItem* Item, const FHhAxialCoord& Location)
{
	FScopedLayoutBatch Batch(this);
	return InventoryArray.AddItem(Item, Location);
}

void UInventoryComponent::RemoveItem(const UInventoryItem* Item)
{
	FScopedLayoutBatch Batch(this);
	InventoryArray.RemoveItem(Item);
}

void UInventoryComponent::RemoveItem(const FHhAxialCoord& Location)
{
	FScopedLayoutBatch Batch(this);
	InventoryArray.RemoveItem(Location);
}

bool UInventoryComponent::MoveItem(const UInventoryItem* Item, const FHhAxialCoord& Location)
{
	FScopedLayoutBatch Batch(this);
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

	// Listeners of both inventories see the whole transfer / swap as one change.
	FScopedLayoutBatch SourceBatch(this);
	FScopedLayoutBatch TargetBatch(Target);

	if (Target == this)
	{
		if (!Occupant)
		{
			return MoveItem(Item, Location);
		}
		return Occupant == Item || InventoryArray.SwapItems(Item, Occupant);
	}

	// Between inventories items are re-created (manifest + instance state): an item object is a replicated
	// subobject of exactly one component.
	UInventoryItem* MovedItem = Item->CloneFor(Target);
	UInventoryItem* ReturnedOccupant = Occupant ? Occupant->CloneFor(this) : nullptr;

	RemoveItem(Item);
	if (Occupant)
	{
		Target->RemoveItem(Occupant);
	}

	const bool bMoved = Target->AddItem(MovedItem, Location);
	if (ReturnedOccupant)
	{
		AddItem(ReturnedOccupant, SourceLocation.GetValue());
	}
	return ensure(bMoved);
}

bool UInventoryComponent::RotateItem(UInventoryItem* Item, const int32 Steps)
{
	const AActor* Owner = GetOwner();
	if (!Item || !Owner || !Owner->HasAuthority() || !GetItemLocation(Item).IsSet())
	{
		return false;
	}

	FScopedLayoutBatch Batch(this);
	Item->SetRotation(Item->GetRotation() + Steps);
	return true;
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

UInventoryComponent::FScopedLayoutBatch::FScopedLayoutBatch(UInventoryComponent* InInventory)
	: Inventory(InInventory)
{
	if (InInventory)
	{
		++InInventory->LayoutBatchDepth;
	}
}

UInventoryComponent::FScopedLayoutBatch::~FScopedLayoutBatch()
{
	if (UInventoryComponent* PinnedInventory = Inventory.Get())
	{
		if (ensure(PinnedInventory->LayoutBatchDepth > 0) && --PinnedInventory->LayoutBatchDepth == 0)
		{
			PinnedInventory->FlushLayoutChange();
		}
	}
}

TArray<UInventoryItem*> UInventoryComponent::GetNeighborItems(const FHhAxialCoord& Location) const
{
	TArray<UInventoryItem*> Result;
	for (const FHhAxialCoord& NeighborLocation : InventoryLayout::GetNeighborLocations(Location))
	{
		if (UInventoryItem* Neighbor = GetItemAt(NeighborLocation))
		{
			Result.Add(Neighbor);
		}
	}
	return Result;
}

TArray<UInventoryItem*> UInventoryComponent::GetItemNeighbors(const UInventoryItem* Item) const
{
	const TOptional<FHhAxialCoord> Location = Item ? GetItemLocation(Item) : TOptional<FHhAxialCoord>();
	return Location.IsSet() ? GetNeighborItems(Location.GetValue()) : TArray<UInventoryItem*>();
}

void UInventoryComponent::FlushLayoutChange()
{
	if (LayoutBatchDepth > 0)
	{
		return;
	}

	InventoryLayout::FLayout Before;
	InventoryLayout::FRotations RotationsBefore;
	for (const TPair<TWeakObjectPtr<UInventoryItem>, FReportedPlacement>& Reported : ReportedLayout)
	{
		if (UInventoryItem* Item = Reported.Key.Get())
		{
			Before.Add(Item, Reported.Value.Location);
			RotationsBefore.Add(Item, Reported.Value.Rotation);
		}
	}

	InventoryLayout::FLayout After;
	InventoryLayout::FRotations RotationsAfter;
	for (const TPair<UInventoryItem*, FHhAxialCoord>& Placed : GetPlacedItems())
	{
		After.Add(Placed.Key, Placed.Value);
		RotationsAfter.Add(Placed.Key, Placed.Key->GetRotation());
	}

	FInventoryLayoutChange Change = InventoryLayout::Diff(Before, After);
	Change.Rotations = InventoryLayout::DiffRotations(RotationsBefore, RotationsAfter);
	if (Change.IsEmpty())
	{
		return;
	}

	ReportedLayout.Reset();
	for (const TPair<UInventoryItem*, FHhAxialCoord>& Placed : After)
	{
		ReportedLayout.Add(Placed.Key, { Placed.Value, RotationsAfter[Placed.Key] });
	}

	LayoutChangedDelegate.Broadcast(this, Change);
	OnLayoutChanged.Broadcast(this, Change);
}
