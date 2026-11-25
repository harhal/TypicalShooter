#include "PowerGrid/SourceAmmoComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "InventoryComponent.h"
#include "InventoryItem.h"
#include "PowerGrid/PowerGridComponent.h"
#include "PowerGrid/PowerGridFragment.h"
#include "Shooting/AmmoAttributeSet.h"

namespace SourceAmmoPrivate
{
	/** Source parameters of an item whose source needs ammo, nullptr otherwise. */
	const FPowerSourceParams* FindAmmoSource(const UInventoryItem* Item)
	{
		const FInventoryItemManifest* Manifest = Item ? Item->GetItemManifest() : nullptr;
		const FPowerGrid_ItemFragment* Fragment = Manifest ? Manifest->GetFragmentOfType<FPowerGrid_ItemFragment>() : nullptr;
		return Fragment && Fragment->ItemType == EPowerItemType::Source && Fragment->Source.NeedsAmmo() ? &Fragment->Source : nullptr;
	}
}

USourceAmmoComponent::USourceAmmoComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USourceAmmoComponent::BeginPlay()
{
	Super::BeginPlay();
	AActor* Owner = GetOwner();
	UInventoryComponent* InventoryPtr = Owner ? Owner->FindComponentByClass<UInventoryComponent>() : nullptr;
	UAbilitySystemComponent* AscPtr = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner);
	if (!InventoryPtr || !AscPtr || !Owner->HasAuthority())
	{
		return;
	}
	Inventory = InventoryPtr;
	Asc = AscPtr;
	AddedHandle = InventoryPtr->GetItemAddedDelegate().AddUObject(this, &ThisClass::HandleItemAdded);
	RemovedHandle = InventoryPtr->GetItemRemovedDelegate().AddUObject(this, &ThisClass::HandleItemRemoved);
	for (const TPair<UInventoryItem*, FHhAxialCoord>& Placed : InventoryPtr->GetPlacedItems())
	{
		HandleItemAdded(Placed.Key, Placed.Value);
	}
}

void USourceAmmoComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UInventoryComponent* InventoryPtr = Inventory.Get())
	{
		InventoryPtr->GetItemAddedDelegate().Remove(AddedHandle);
		InventoryPtr->GetItemRemovedDelegate().Remove(RemovedHandle);
	}
	Super::EndPlay(EndPlayReason);
}

void USourceAmmoComponent::HandleItemAdded(const UInventoryItem* Item, const FHhAxialCoord& Location)
{
	// Delegates carry const items; the clip is this component's state of items placed in its own inventory.
	Refill(const_cast<UInventoryItem*>(Item));
}

void USourceAmmoComponent::HandleItemRemoved(const UInventoryItem* Item)
{
	const FPowerSourceParams* Source = SourceAmmoPrivate::FindAmmoSource(Item);
	UAbilitySystemComponent* AscPtr = Asc.Get();
	const FGameplayAttribute Reserve = Source ? UAmmoAttributeSet::GetReserveAttribute(Source->AmmoType) : FGameplayAttribute();
	if (!AscPtr || !Reserve.IsValid() || !AscPtr->HasAttributeSetForAttribute(Reserve) || Item->GetClip() <= 0)
	{
		return;
	}
	AscPtr->SetNumericAttributeBase(Reserve, AscPtr->GetNumericAttributeBase(Reserve) + Item->GetClip());
	const_cast<UInventoryItem*>(Item)->SetClip(0);
}

void USourceAmmoComponent::Refill(UInventoryItem* Item) const
{
	const FPowerSourceParams* Source = SourceAmmoPrivate::FindAmmoSource(Item);
	UAbilitySystemComponent* AscPtr = Asc.Get();
	const FGameplayAttribute Reserve = Source ? UAmmoAttributeSet::GetReserveAttribute(Source->AmmoType) : FGameplayAttribute();
	if (!AscPtr || !Reserve.IsValid() || !AscPtr->HasAttributeSetForAttribute(Reserve))
	{
		return;
	}
	const int32 Available = FMath::FloorToInt32(AscPtr->GetNumericAttributeBase(Reserve));
	const int32 Taken = FMath::Clamp(Source->Capacity - Item->GetClip(), 0, Available);
	if (Taken > 0)
	{
		AscPtr->SetNumericAttributeBase(Reserve, AscPtr->GetNumericAttributeBase(Reserve) - Taken);
		Item->SetClip(Item->GetClip() + Taken);
	}
}

void USourceAmmoComponent::ReportUse(const UInventoryItem* Receiver)
{
	const UPowerGridComponent* Grid = GetOwner() ? GetOwner()->FindComponentByClass<UPowerGridComponent>() : nullptr;
	if (!Grid || !Asc.IsValid())
	{
		return;
	}
	// Collected first: each clip change re-solves the grid (an emptied source switches off).
	for (UInventoryItem* Source : Grid->GetFeedingSources(Receiver))
	{
		if (const FPowerSourceParams* Params = SourceAmmoPrivate::FindAmmoSource(Source))
		{
			Source->SetClip(Source->GetClip() - FMath::Max(1, Params->CostPerUse));
		}
	}
}
