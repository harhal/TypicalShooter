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

	/** The air slot works only while the avatar character is airborne (power grid R5). */
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
		FGameplayTagContainer* OptionalRelevantTags) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	void StartBurst();
	void FireNextShot();
	void OnBurstIntervalElapsed();

	/** Spawns the pattern's projectiles (attacks have no clip, power grid R9a). */
	void FireShot();
	/** Spawns the shot's projectiles; returns the shot direction (centre of the pattern). */
	FVector SpawnProjectiles(const FAttackStats& Stats) const;
	/** Pushes the avatar character opposite to the shot by Recoil cm/s (power grid R5c). */
	void ApplyRecoil(const FVector& ShotDirection, float Recoil) const;
	/** Tells the power sources feeding this ability's receiver that it was used once (power grid R9b). */
	void ReportUse() const;

	/** Pays the held items' ammo-gated modifiers for this shot and adds their bonuses to Stats (R15a). */
	void ApplyShotModifiers(UAbilitySystemComponent& Asc, FAttackStats& Stats) const;
	void Finish();
	void SetTimer(float Delay, void (UAttackGameplayAbility::*Callback)());

	int32 ShotsLeftInBurst = 0;
	FTimerHandle Timer;
};
