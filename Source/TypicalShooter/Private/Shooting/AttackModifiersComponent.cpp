#include "Shooting/AttackModifiersComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "InventoryComponent.h"
#include "InventoryItem.h"
#include "InventoryItemManifest.h"
#include "Shooting/AttackModifierFragment.h"

UAttackModifiersComponent::UAttackModifiersComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

const FShotPattern* UAttackModifiersComponent::FindPatternOverride(const EAttackSlot Slot) const
{
	for (int32 Index = PatternOverrides.Num() - 1; Index >= 0; --Index)
	{
		if (PatternOverrides[Index].Slot == Slot)
		{
			return &PatternOverrides[Index].Pattern;
		}
	}
	return nullptr;
}

void UAttackModifiersComponent::BeginPlay()
{
	Super::BeginPlay();
	UInventoryComponent* Inventory = GetInventory();
	if (!Inventory || !GetOwner()->HasAuthority())
	{
		return;
	}
	AddedHandle = Inventory->GetItemAddedDelegate().AddUObject(this, &UAttackModifiersComponent::HandleItemAdded);
	RemovedHandle = Inventory->GetItemRemovedDelegate().AddUObject(this, &UAttackModifiersComponent::HandleItemRemoved);
	for (const TPair<UInventoryItem*, FHhAxialCoord>& Placed : Inventory->GetPlacedItems())
	{
		HandleItemAdded(Placed.Key, Placed.Value);
	}
}

void UAttackModifiersComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UInventoryComponent* Inventory = GetInventory())
	{
		Inventory->GetItemAddedDelegate().Remove(AddedHandle);
		Inventory->GetItemRemovedDelegate().Remove(RemovedHandle);
	}
	Super::EndPlay(EndPlayReason);
}

void UAttackModifiersComponent::HandleItemAdded(const UInventoryItem* Item, const FHhAxialCoord& Location)
{
	const FInventoryItemManifest* Manifest = Item ? Item->GetItemManifest() : nullptr;
	const FAttackModifier_ItemFragment* Fragment = Manifest ? Manifest->GetFragmentOfType<FAttackModifier_ItemFragment>() : nullptr;
	if (!Fragment || AppliedEffects.Contains(Item))
	{
		return;
	}

	UAbilitySystemComponent* Asc = GetAbilitySystem();
	FActiveGameplayEffectHandle Handle;
	if (Asc && Fragment->Effect)
	{
		FGameplayEffectContextHandle Context = Asc->MakeEffectContext();
		Context.AddSourceObject(Item);
		Handle = Asc->ApplyGameplayEffectToSelf(Fragment->Effect.GetDefaultObject(), 1.f, Context);
	}
	AppliedEffects.Add(Item, Handle);

	if (Fragment->bReplacePattern)
	{
		PatternOverrides.Add({ Item, Fragment->PatternSlot, Fragment->Pattern });
	}
}

void UAttackModifiersComponent::HandleItemRemoved(const UInventoryItem* Item)
{
	FActiveGameplayEffectHandle Handle;
	if (!AppliedEffects.RemoveAndCopyValue(Item, Handle))
	{
		return;
	}
	UAbilitySystemComponent* Asc = GetAbilitySystem();
	if (Asc && Handle.IsValid())
	{
		Asc->RemoveActiveGameplayEffect(Handle);
	}
	const TObjectKey<UInventoryItem> Key(Item);
	PatternOverrides.RemoveAll([&Key](const FPatternOverride& Override) { return Override.Item == Key; });
}

UInventoryComponent* UAttackModifiersComponent::GetInventory() const
{
	return GetOwner() ? GetOwner()->FindComponentByClass<UInventoryComponent>() : nullptr;
}

UAbilitySystemComponent* UAttackModifiersComponent::GetAbilitySystem() const
{
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
}
