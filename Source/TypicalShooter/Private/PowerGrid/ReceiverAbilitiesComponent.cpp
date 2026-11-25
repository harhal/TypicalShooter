#include "PowerGrid/ReceiverAbilitiesComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "InventoryItem.h"
#include "PowerGrid/PowerGridComponent.h"
#include "PowerGrid/PowerGridFragment.h"
#include "Shooting/AttackGameplayAbility.h"

UReceiverAbilitiesComponent::UReceiverAbilitiesComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

EAttackSlot UReceiverAbilitiesComponent::ToAttackSlot(const EReceiverSlot Slot)
{
	switch (Slot)
	{
	case EReceiverSlot::Primary:   return EAttackSlot::Primary;
	case EReceiverSlot::Alternate: return EAttackSlot::Alternate;
	case EReceiverSlot::Air:       return EAttackSlot::Air;
	}
	checkNoEntry();
	return EAttackSlot::Primary;
}

EAttackInputID UReceiverAbilitiesComponent::GetInput(const EReceiverSlot Slot)
{
	switch (Slot)
	{
	case EReceiverSlot::Primary:   return EAttackInputID::PrimaryAttack;
	case EReceiverSlot::Alternate: return EAttackInputID::AlternateAttack;
	case EReceiverSlot::Air:       return EAttackInputID::Jump;
	}
	checkNoEntry();
	return EAttackInputID::None;
}

void UReceiverAbilitiesComponent::BeginPlay()
{
	Super::BeginPlay();
	AActor* Owner = GetOwner();
	UPowerGridComponent* Grid = Owner ? Owner->FindComponentByClass<UPowerGridComponent>() : nullptr;
	UAbilitySystemComponent* AscPtr = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner);
	if (!Grid || !AscPtr || !Owner->HasAuthority())
	{
		return;
	}
	PowerGrid = Grid;
	Asc = AscPtr;
	GridChangedHandle = Grid->GetPowerGridChangedDelegate().AddUObject(this, &ThisClass::HandleGridChanged);
	Sync();
}

void UReceiverAbilitiesComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UPowerGridComponent* Grid = PowerGrid.Get())
	{
		Grid->GetPowerGridChangedDelegate().Remove(GridChangedHandle);
	}
	for (FSlotGrant& SlotGrant : Grants)
	{
		Revoke(SlotGrant);
	}
	Super::EndPlay(EndPlayReason);
}

void UReceiverAbilitiesComponent::HandleGridChanged(UPowerGridComponent* Grid)
{
	Sync();
}

void UReceiverAbilitiesComponent::Sync()
{
	const UPowerGridComponent* Grid = PowerGrid.Get();
	for (const EReceiverSlot Slot : ReceiverSlots::GetAll())
	{
		SyncSlot(Slot, Grid ? Grid->GetActiveReceiver(Slot) : nullptr);
	}
}

void UReceiverAbilitiesComponent::SyncSlot(const EReceiverSlot Slot, UInventoryItem* Active)
{
	FSlotGrant& SlotGrant = Grants[static_cast<int32>(Slot)];
	const bool bUnchanged = Active ? SlotGrant.Receiver.Get() == Active : !SlotGrant.Handle.IsValid();
	if (bUnchanged)
	{
		return;
	}
	Revoke(SlotGrant);
	if (Active)
	{
		Grant(Slot, *Active, SlotGrant);
	}
}

void UReceiverAbilitiesComponent::Revoke(FSlotGrant& SlotGrant) const
{
	if (UAbilitySystemComponent* AscPtr = Asc.Get(); AscPtr && SlotGrant.Handle.IsValid())
	{
		AscPtr->ClearAbility(SlotGrant.Handle);
	}
	SlotGrant = FSlotGrant();
}

void UReceiverAbilitiesComponent::Grant(const EReceiverSlot Slot, UInventoryItem& Receiver, FSlotGrant& OutGrant) const
{
	UAbilitySystemComponent* AscPtr = Asc.Get();
	const FInventoryItemManifest* Manifest = Receiver.GetItemManifest();
	const FPowerGrid_ItemFragment* Fragment = Manifest ? Manifest->GetFragmentOfType<FPowerGrid_ItemFragment>() : nullptr;
	OutGrant.Receiver = &Receiver;
	if (!AscPtr || !Fragment || !Fragment->Receiver.Ability)
	{
		return;
	}
	OutGrant.Handle = AscPtr->GiveAbility(FGameplayAbilitySpec(Fragment->Receiver.Ability, 1, static_cast<int32>(GetInput(Slot)), &Receiver));
	// One ability class serves every slot: the slot only picks the attributes, trigger and direction (R5b).
	const FGameplayAbilitySpec* Spec = AscPtr->FindAbilitySpecFromHandle(OutGrant.Handle);
	if (UAttackGameplayAbility* Attack = Spec ? Cast<UAttackGameplayAbility>(Spec->GetPrimaryInstance()) : nullptr)
	{
		Attack->Slot = ToAttackSlot(Slot);
	}
}
