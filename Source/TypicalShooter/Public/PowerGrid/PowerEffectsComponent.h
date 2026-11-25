// Applies the threshold effects of the owner's items (power grid R13-R14a, R13a): on every power grid change, each
// reached step's character effect is active, and the attack bonuses of the items connected to each active receiver
// are added to that receiver slot's attributes (one UAttackBonusEffect). Server only; needs an ASC on the owner.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "PowerEffectsComponent.generated.h"

class UAbilitySystemComponent;
class UInventoryItem;
class UPowerGridComponent;

UCLASS(ClassGroup = (PowerGrid), meta = (BlueprintSpawnableComponent))
class TYPICALSHOOTER_API UPowerEffectsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPowerEffectsComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

private:
	/** A reached step of an item: the item and the index of the step in its fragment. */
	using FStepKey = TPair<TObjectKey<UInventoryItem>, int32>;
	/** Bonus per [slot * attribute count + attack parameter]. */
	using FBonusTable = TArray<float>;

	void Sync();
	void SyncCharacterEffects(UAbilitySystemComponent& InAsc, const UPowerGridComponent& Grid);
	void SyncAttackBonuses(UAbilitySystemComponent& InAsc, const UPowerGridComponent& Grid);
	void ApplyAttackBonuses(UAbilitySystemComponent& InAsc);
	void RemoveAll();
	void HandleGridChanged(UPowerGridComponent* Grid);

	static FBonusTable CollectAttackBonuses(const UPowerGridComponent& Grid);

	TWeakObjectPtr<UPowerGridComponent> PowerGrid;
	TWeakObjectPtr<UAbilitySystemComponent> Asc;
	FDelegateHandle GridChangedHandle;

	TMap<FStepKey, FActiveGameplayEffectHandle> CharacterEffects;
	FActiveGameplayEffectHandle AttackBonusHandle;
	FBonusTable AppliedBonuses;
};
