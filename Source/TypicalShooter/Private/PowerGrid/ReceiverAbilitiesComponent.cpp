#include "PowerGrid/ReceiverAbilitiesComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "InventoryItem.h"
#include "PowerGrid/PowerGridComponent.h"
#include "PowerGrid/PowerGridFragment.h"
#include "Shooting/AttackAttributeSet.h"
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
	for (const EReceiverSlot Slot : ReceiverSlots::GetAll())
	{
		Revoke(Slot);
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
	const FSlotGrant& SlotGrant = Grants[static_cast<int32>(Slot)];
	const bool bUnchanged = Active ? SlotGrant.bGranted && SlotGrant.Receiver.Get() == Active : !SlotGrant.bGranted;
	if (bUnchanged)
	{
		return;
	}
	Revoke(Slot);
	if (Active)
	{
		Grant(Slot, *Active);
	}
}

void UReceiverAbilitiesComponent::Revoke(const EReceiverSlot Slot)
{
	FSlotGrant& SlotGrant = Grants[static_cast<int32>(Slot)];
	UAbilitySystemComponent* AscPtr = Asc.Get();
	if (AscPtr && SlotGrant.Handle.IsValid())
	{
		AscPtr->ClearAbility(SlotGrant.Handle);
	}
	if (AscPtr && SlotGrant.bGranted)
	{
		SetSlotBase(*AscPtr, ToAttackSlot(Slot), {});
	}
	SlotGrant = FSlotGrant();
}

void UReceiverAbilitiesComponent::Grant(const EReceiverSlot Slot, UInventoryItem& Receiver)
{
	FSlotGrant& SlotGrant = Grants[static_cast<int32>(Slot)];
	SlotGrant.Receiver = &Receiver;
	SlotGrant.bGranted = true;
	UAbilitySystemComponent* AscPtr = Asc.Get();
	const FInventoryItemManifest* Manifest = Receiver.GetItemManifest();
	const FPowerGrid_ItemFragment* Fragment = Manifest ? Manifest->GetFragmentOfType<FPowerGrid_ItemFragment>() : nullptr;
	if (!AscPtr || !Fragment)
	{
		return;
	}
	SetSlotBase(*AscPtr, ToAttackSlot(Slot), Fragment->Receiver.BaseStats);
	if (!Fragment->Receiver.Ability)
	{
		return;
	}
	SlotGrant.Handle = AscPtr->GiveAbility(FGameplayAbilitySpec(Fragment->Receiver.Ability, 1, static_cast<int32>(GetInput(Slot)), &Receiver));
	// One ability class serves every slot: the slot only picks the attributes, trigger and direction (R5b).
	const FGameplayAbilitySpec* Spec = AscPtr->FindAbilitySpecFromHandle(SlotGrant.Handle);
	if (UAttackGameplayAbility* Attack = Spec ? Cast<UAttackGameplayAbility>(Spec->GetPrimaryInstance()) : nullptr)
	{
		Attack->Slot = ToAttackSlot(Slot);
	}
}

void UReceiverAbilitiesComponent::SetSlotBase(UAbilitySystemComponent& InAsc, const EAttackSlot Slot, const TArray<FAttackStatBonus>& BaseStats)
{
	if (!InAsc.HasAttributeSetForAttribute(UAttackAttributeSet::GetAttribute(Slot, EAttackAttribute::DamagePhysical)))
	{
		return;
	}
	// Neutral defaults first, then the receiver's own values (R5a); effects of items stack on top of the base.
	for (int32 Index = 0; Index < static_cast<int32>(EAttackAttribute::Count); ++Index)
	{
		const EAttackAttribute Attribute = static_cast<EAttackAttribute>(Index);
		InAsc.SetNumericAttributeBase(UAttackAttributeSet::GetAttribute(Slot, Attribute), UAttackAttributeSet::GetDefaultValue(Attribute));
	}
	for (const FAttackStatBonus& Stat : BaseStats)
	{
		InAsc.SetNumericAttributeBase(UAttackAttributeSet::GetAttribute(Slot, Stat.Attribute), Stat.Value);
	}
}
