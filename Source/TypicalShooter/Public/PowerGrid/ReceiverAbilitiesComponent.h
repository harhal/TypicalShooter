// Grants the abilities of the active receivers (power grid R5, R5b, R6, R6a): whenever the owner's power grid changes,
// each receiver slot gets the ability of its active receiver (or none), bound to the slot's input. Server only.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayAbilitySpecHandle.h"
#include "PowerGrid/ReceiverSlots.h"
#include "Shooting/AttackTypes.h"
#include "Shooting/ShotModifier.h"
#include "ReceiverAbilitiesComponent.generated.h"

class UAbilitySystemComponent;
class UInventoryItem;
class UPowerGridComponent;

UCLASS(ClassGroup = (PowerGrid), meta = (BlueprintSpawnableComponent))
class TYPICALSHOOTER_API UReceiverAbilitiesComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UReceiverAbilitiesComponent();

	/** Attack slot whose attributes a receiver slot uses (same order). */
	static EAttackSlot ToAttackSlot(EReceiverSlot Slot);
	/** Input of a receiver slot: attack buttons; the air slot uses the jump button (only works while airborne). */
	static EAttackInputID GetInput(EReceiverSlot Slot);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

private:
	struct FSlotGrant
	{
		TWeakObjectPtr<UInventoryItem> Receiver;
		FGameplayAbilitySpecHandle Handle;
		/** A receiver works in the slot (its base stats are applied), even if it grants no ability. */
		bool bGranted = false;
	};

	void Sync();
	void SyncSlot(EReceiverSlot Slot, UInventoryItem* Active);
	/** Takes the slot's ability away and resets its attack attributes to neutral. */
	void Revoke(EReceiverSlot Slot);
	/** Applies the receiver's base stats to the slot and grants its ability with the slot's input. */
	void Grant(EReceiverSlot Slot, UInventoryItem& Receiver);
	void HandleGridChanged(UPowerGridComponent* Grid);

	/** Base values of Slot's attack attributes: neutral defaults overridden by BaseStats (R5a). */
	static void SetSlotBase(UAbilitySystemComponent& InAsc, EAttackSlot Slot, const TArray<FAttackStatBonus>& BaseStats);

	TWeakObjectPtr<UPowerGridComponent> PowerGrid;
	TWeakObjectPtr<UAbilitySystemComponent> Asc;
	FDelegateHandle GridChangedHandle;
	TStaticArray<FSlotGrant, 3> Grants;
};
