// Enemy shooter: GAS health and the same projectile attack as the player (primary slot), with its own weaker base
// stats. The AI decides where to stand and when to fire (SetFiring).

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "Enemies/Coordination/EnemyRole.h"
#include "Enemies/Coordination/EnemyRoleStand.h"
#include "Shooting/ShotModifier.h"
#include "EnemyCharacter.generated.h"

class UAttackAttributeSet;
class UAttackGameplayAbility;
class UGasDamageReceiverComponent;
class UHealthAttributeSet;
class UHealthComponent;
class AEnemyCharacter;

DECLARE_MULTICAST_DELEGATE_OneParam(FEnemyDiedNative, AEnemyCharacter*);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEnemyDiedDynamic, AEnemyCharacter*, Enemy);

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

	/** Target within the attack's fire end distance and a Visibility trace from the muzzle reaches it unblocked. */
	UFUNCTION(BlueprintPure, Category = "Enemy|Attack")
	bool CanShootAt(const AActor* Target) const;

	/**
	 * Could this enemy's attack reach Target right now (the coordinator hands attack tokens only to such enemies)? A shooter
	 * checks CanShootAt; melee / kamikaze enemies override it with their own reach.
	 */
	virtual bool CanAttack(const AActor* Target) const { return CanShootAt(Target); }

	/** Roles the coordinator may give this enemy; empty = any. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Coordination")
	TArray<EEnemyRole> AllowedRoles = {EEnemyRole::Pressure, EEnemyRole::Flank, EEnemyRole::Cover};

	/** Distance bands of this enemy class per role; roles without an entry use the project's bands. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Coordination")
	TMap<EEnemyRole, FEnemyDistanceBand> RoleBandOverrides;

	/** EnemyRoles::Mask bits of AllowedRoles (0 = any). */
	uint8 GetAllowedRoleMask() const;

	/** Band for Role: this class's override, else UEnemyCoordinationSettings::RoleBands. */
	FEnemyDistanceBand GetRoleBand(EEnemyRole ForRole) const;

	/** Server: fire while CanShootAt(Target), otherwise release the attack. */
	UFUNCTION(BlueprintCallable, Category = "Enemy|Attack")
	void UpdateFiring(const AActor* Target);

	/** Seconds the dead body stays before it is destroyed. */
	UPROPERTY(EditDefaultsOnly, Category = "Enemy", meta = (ClampMin = 0.01))
	float DeathLifeSpan = 0.1f;

	/** Server: health reached zero; the enemy stopped and will disappear after DeathLifeSpan. */
	UPROPERTY(BlueprintAssignable, Category = "Enemy")
	FEnemyDiedDynamic OnDied;

	FEnemyDiedNative& GetDiedDelegate() { return DiedDelegate; }

	UFUNCTION(BlueprintPure, Category = "Enemy")
	bool IsDead() const { return bDead; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void GrantAttack();
	void Die();

	FEnemyDiedNative DiedDelegate;
	bool bDead = false;

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
