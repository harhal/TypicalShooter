// Reload of one attack slot: after the slot's reload time moves rounds of AmmoType from the reserve
// (UAmmoAttributeSet) into the clip. The slot's attack can't fire meanwhile; no reserve, no reload.

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

	/** Ammo kind loaded into this slot's clip (Ammo.*), taken from UAmmoAttributeSet. None = unlimited reserve. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack", meta = (Categories = "Ammo"))
	FGameplayTag AmmoType;

	/** Activates the reload ability of Slot granted to Asc, if any. Server. */
	static bool TryReload(UAbilitySystemComponent& Asc, EAttackSlot Slot);

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	void CompleteReload();
	void Finish();

	/** Reserve attribute of AmmoType if Asc has it; invalid = unlimited reserve. */
	FGameplayAttribute GetReserveAttribute(const UAbilitySystemComponent& Asc) const;

	/** Rounds available for this reload (MAX_int32 when unlimited). */
	int32 GetReserve(const UAbilitySystemComponent& Asc) const;

	FTimerHandle Timer;
	bool bReloadingTagAdded = false;
};
