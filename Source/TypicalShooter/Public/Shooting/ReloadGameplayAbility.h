// Reload of one attack slot: after the slot's reload time fills the clip. The slot's attack can't fire meanwhile.
// Reserve ammo is unlimited for now.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Shooting/AttackTypes.h"
#include "ReloadGameplayAbility.generated.h"

UCLASS()
class TYPICALSHOOTER_API UReloadGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UReloadGameplayAbility();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	EAttackSlot Slot = EAttackSlot::Primary;

	/** Activates the reload ability of Slot granted to Asc, if any. Server. */
	static bool TryReload(UAbilitySystemComponent& Asc, EAttackSlot Slot);

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	void CompleteReload();
	void Finish();

	FTimerHandle Timer;
	bool bReloadingTagAdded = false;
};
