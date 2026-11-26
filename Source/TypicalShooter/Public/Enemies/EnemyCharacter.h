// Enemy shooter: GAS health and the same projectile attack as the player (primary slot), with its own weaker base
// stats. The AI decides where to stand and when to fire (SetFiring).

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "Shooting/ShotModifier.h"
#include "EnemyCharacter.generated.h"

class UAttackAttributeSet;
class UAttackGameplayAbility;
class UGasDamageReceiverComponent;
class UHealthAttributeSet;
class UHealthComponent;

UCLASS()
class TYPICALSHOOTER_API AEnemyCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AEnemyCharacter();

	/** Attack ability granted on the server (primary slot). */
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Attack")
	TSubclassOf<UAttackGameplayAbility> AttackAbility;

	/** Base values of the primary attack attributes (the rest stay neutral). */
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Attack")
	TArray<FAttackStatBonus> AttackStats;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UHealthComponent* GetHealth() const { return Health; }

	/** Server: hold / release the attack button. While held, the attack repeats bursts. */
	UFUNCTION(BlueprintCallable, Category = "Enemy|Attack")
	void SetFiring(bool bFire);

	UFUNCTION(BlueprintPure, Category = "Enemy|Attack")
	bool IsFiring() const { return bFiring; }

protected:
	virtual void BeginPlay() override;

private:
	void GrantAttack();

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UAbilitySystemComponent> Gas;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHealthComponent> Health;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UGasDamageReceiverComponent> DamageReceiver;

	/** Projectiles start here (tagged "Muzzle"). */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> Muzzle;

	UPROPERTY()
	TObjectPtr<UHealthAttributeSet> HealthAttributes;

	UPROPERTY()
	TObjectPtr<UAttackAttributeSet> AttackAttributes;

	bool bFiring = false;
};
