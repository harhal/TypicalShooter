#include "PowerGrid/PowerEffectsComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "InventoryComponent.h"
#include "InventoryItem.h"
#include "PowerGrid/AttackBonusEffect.h"
#include "PowerGrid/PowerEffectsFragment.h"
#include "PowerGrid/PowerGridComponent.h"
#include "PowerGrid/ReceiverAbilitiesComponent.h"

namespace PowerEffectsPrivate
{
	constexpr int32 NumSlots = static_cast<int32>(EAttackSlot::Air) + 1;
	constexpr int32 NumAttributes = static_cast<int32>(EAttackAttribute::Count);

	const FPowerEffects_ItemFragment* FindFragment(const UInventoryItem* Item)
	{
		const FInventoryItemManifest* Manifest = Item ? Item->GetItemManifest() : nullptr;
		return Manifest ? Manifest->GetFragmentOfType<FPowerEffects_ItemFragment>() : nullptr;
	}

	/** Calls Function(StepIndex, Step) for every reached step of Item: threshold <= power fed to it (R13). */
	template <typename FunctionType>
	void ForEachReachedStep(const UPowerGridComponent& Grid, const UInventoryItem* Item, FunctionType&& Function)
	{
		const FPowerEffects_ItemFragment* Fragment = FindFragment(Item);
		if (!Fragment)
		{
			return;
		}
		const int32 Power = Grid.GetItemPower(Item);
		for (int32 Index = 0; Index < Fragment->Effects.Num(); ++Index)
		{
			if (Fragment->Effects[Index].Threshold <= Power)
			{
				Function(Index, Fragment->Effects[Index]);
			}
		}
	}
}

UPowerEffectsComponent::UPowerEffectsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPowerEffectsComponent::BeginPlay()
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

void UPowerEffectsComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UPowerGridComponent* Grid = PowerGrid.Get())
	{
		Grid->GetPowerGridChangedDelegate().Remove(GridChangedHandle);
	}
	RemoveAll();
	Super::EndPlay(EndPlayReason);
}

void UPowerEffectsComponent::HandleGridChanged(UPowerGridComponent* Grid)
{
	Sync();
}

void UPowerEffectsComponent::Sync()
{
	UAbilitySystemComponent* AscPtr = Asc.Get();
	const UPowerGridComponent* Grid = PowerGrid.Get();
	if (AscPtr && Grid)
	{
		SyncCharacterEffects(*AscPtr, *Grid);
		SyncAttackBonuses(*AscPtr, *Grid);
	}
}

void UPowerEffectsComponent::SyncCharacterEffects(UAbilitySystemComponent& InAsc, const UPowerGridComponent& Grid)
{
	// Steps that stay reached keep their effect: removing and re-applying could clamp e.g. health.
	TMap<FStepKey, FActiveGameplayEffectHandle> Next;
	const UInventoryComponent* Inventory = Grid.GetInventory();
	const TArray<TPair<UInventoryItem*, FHhAxialCoord>> Placed = Inventory ? Inventory->GetPlacedItems() : TArray<TPair<UInventoryItem*, FHhAxialCoord>>();
	for (const TPair<UInventoryItem*, FHhAxialCoord>& Entry : Placed)
	{
		UInventoryItem* Item = Entry.Key;
		PowerEffectsPrivate::ForEachReachedStep(Grid, Item, [&](const int32 Index, const FPowerThresholdEffect& Step)
		{
			if (!Step.CharacterEffect)
			{
				return;
			}
			const FStepKey Key(Item, Index);
			FActiveGameplayEffectHandle Handle;
			if (!CharacterEffects.RemoveAndCopyValue(Key, Handle))
			{
				FGameplayEffectContextHandle Context = InAsc.MakeEffectContext();
				Context.AddSourceObject(Item);
				Handle = InAsc.ApplyGameplayEffectToSelf(Step.CharacterEffect.GetDefaultObject(), Step.CharacterEffectLevel, Context);
			}
			Next.Add(Key, Handle);
		});
	}
	for (const TPair<FStepKey, FActiveGameplayEffectHandle>& Stale : CharacterEffects)
	{
		InAsc.RemoveActiveGameplayEffect(Stale.Value);
	}
	CharacterEffects = MoveTemp(Next);
}

UPowerEffectsComponent::FBonusTable UPowerEffectsComponent::CollectAttackBonuses(const UPowerGridComponent& Grid)
{
	using namespace PowerEffectsPrivate;
	FBonusTable Bonuses;
	Bonuses.SetNumZeroed(NumSlots * NumAttributes);
	for (const EReceiverSlot Slot : ReceiverSlots::GetAll())
	{
		const UInventoryItem* Receiver = Grid.GetActiveReceiver(Slot);
		if (!Receiver)
		{
			continue;
		}
		const int32 SlotOffset = static_cast<int32>(UReceiverAbilitiesComponent::ToAttackSlot(Slot)) * NumAttributes;
		for (const UInventoryItem* Item : Grid.GetConnectedItems(Receiver))
		{
			ForEachReachedStep(Grid, Item, [&](int32, const FPowerThresholdEffect& Step)
			{
				for (const FAttackStatBonus& Bonus : Step.AttackBonuses)
				{
					Bonuses[SlotOffset + static_cast<int32>(Bonus.Attribute)] += Bonus.Value;
				}
			});
		}
	}
	return Bonuses;
}

void UPowerEffectsComponent::SyncAttackBonuses(UAbilitySystemComponent& InAsc, const UPowerGridComponent& Grid)
{
	FBonusTable Bonuses = CollectAttackBonuses(Grid);
	if (Bonuses == AppliedBonuses)
	{
		return;
	}
	if (AttackBonusHandle.IsValid())
	{
		InAsc.RemoveActiveGameplayEffect(AttackBonusHandle);
		AttackBonusHandle.Invalidate();
	}
	AppliedBonuses = MoveTemp(Bonuses);
	if (AppliedBonuses.ContainsByPredicate([](const float Value) { return Value != 0.f; }))
	{
		ApplyAttackBonuses(InAsc);
	}
}

void UPowerEffectsComponent::ApplyAttackBonuses(UAbilitySystemComponent& InAsc)
{
	using namespace PowerEffectsPrivate;
	const FGameplayEffectSpecHandle Spec = InAsc.MakeOutgoingSpec(UAttackBonusEffect::StaticClass(), 1.f, InAsc.MakeEffectContext());
	// Every magnitude is set (zeros included): an unset SetByCaller logs an error.
	for (int32 Slot = 0; Slot < NumSlots; ++Slot)
	{
		for (int32 Index = 0; Index < NumAttributes; ++Index)
		{
			const FName Name = UAttackBonusEffect::GetMagnitudeName(static_cast<EAttackSlot>(Slot), static_cast<EAttackAttribute>(Index));
			Spec.Data->SetSetByCallerMagnitude(Name, AppliedBonuses[Slot * NumAttributes + Index]);
		}
	}
	AttackBonusHandle = InAsc.ApplyGameplayEffectSpecToSelf(*Spec.Data);
}

void UPowerEffectsComponent::RemoveAll()
{
	UAbilitySystemComponent* AscPtr = Asc.Get();
	if (!AscPtr)
	{
		return;
	}
	for (const TPair<FStepKey, FActiveGameplayEffectHandle>& Applied : CharacterEffects)
	{
		AscPtr->RemoveActiveGameplayEffect(Applied.Value);
	}
	CharacterEffects.Reset();
	if (AttackBonusHandle.IsValid())
	{
		AscPtr->RemoveActiveGameplayEffect(AttackBonusHandle);
		AttackBonusHandle.Invalidate();
	}
	AppliedBonuses.Reset();
}
