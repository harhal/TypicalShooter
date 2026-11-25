// Primary / alternate attack: fires bursts of projectile shots using the slot's attack attributes.
// Runs on the server; stays active through the burst interval and repeats bursts while input is held.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Shooting/AttackTypes.h"
#include "Shooting/ShotPattern.h"
#include "AttackGameplayAbility.generated.h"

class AShotProjectile;

UCLASS()
class TYPICALSHOOTER_API UAttackGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAttackGameplayAbility();

	/** Which attack attributes this ability uses. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	EAttackSlot Slot = EAttackSlot::Primary;

	/** One projectile per point; items may replace it later. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	FShotPattern Pattern;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	TSubclassOf<AShotProjectile> ProjectileClass;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	void StartBurst();
	void FireNextShot();
	void OnBurstIntervalElapsed();

	/** Spends one round and spawns the pattern's projectiles. False if the clip is empty. */
	bool FireShot();
	void SpawnProjectiles(const FAttackStats& Stats) const;
	void Finish();
	void SetTimer(float Delay, void (UAttackGameplayAbility::*Callback)());

	int32 ShotsLeftInBurst = 0;
	FTimerHandle Timer;
};
