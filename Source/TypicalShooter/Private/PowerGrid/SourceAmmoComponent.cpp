#include "PowerGrid/SourceAmmoComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "InventoryComponent.h"
#include "InventoryItem.h"
#include "PowerGrid/PowerGridComponent.h"
#include "PowerGrid/PowerGridFragment.h"
#include "Shooting/AmmoAttributeSet.h"
#include "TimerManager.h"

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
	// Also reached from BeginDestroy, when the world may already be gone.
	if (const UWorld* World = GetWorld())
	{
		for (TPair<TObjectKey<UInventoryItem>, FTimerHandle>& Recharge : RechargeTimers)
		{
			World->GetTimerManager().ClearTimer(Recharge.Value);
		}
	}
	RechargeTimers.Reset();
	Super::EndPlay(EndPlayReason);
}

void USourceAmmoComponent::HandleItemAdded(const UInventoryItem* Item, const FHhAxialCoord& Location)
{
	// Delegates carry const items; the clip is this component's state of items placed in its own inventory.
	UInventoryItem* Placed = const_cast<UInventoryItem*>(Item);
	Refill(Placed);
	RestartRecharge(Placed); // a short clip keeps trying while the reserve lacks ammo (R9c)
}

void USourceAmmoComponent::HandleItemRemoved(const UInventoryItem* Item)
{
	StopRecharge(Item);
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
			RestartRecharge(Source);
		}
	}
}

void USourceAmmoComponent::RestartRecharge(UInventoryItem* Item)
{
	const FPowerSourceParams* Source = SourceAmmoPrivate::FindAmmoSource(Item);
	if (!Source || !Asc.IsValid() || !GetWorld() || Item->GetClip() >= Source->Capacity)
	{
		StopRecharge(Item);
		return;
	}
	// Looping: after each idle period the clip is topped up; it stops once full (a lacking reserve is retried).
	FTimerHandle& Timer = RechargeTimers.FindOrAdd(Item);
	const TWeakObjectPtr<UInventoryItem> WeakItem(Item);
	GetWorld()->GetTimerManager().SetTimer(Timer, FTimerDelegate::CreateWeakLambda(this, [this, WeakItem]
	{
		if (UInventoryItem* Recharged = WeakItem.Get())
		{
			Refill(Recharged);
			const FPowerSourceParams* Params = SourceAmmoPrivate::FindAmmoSource(Recharged);
			if (!Params || Recharged->GetClip() >= Params->Capacity)
			{
				StopRecharge(Recharged);
			}
		}
	}), FMath::Max(Source->RechargeDelay, MinRechargeDelay), true);
}

void USourceAmmoComponent::StopRecharge(const UInventoryItem* Item)
{
	FTimerHandle Timer;
	const UWorld* World = GetWorld();
	if (RechargeTimers.RemoveAndCopyValue(Item, Timer) && World)
	{
		World->GetTimerManager().ClearTimer(Timer);
	}
}
