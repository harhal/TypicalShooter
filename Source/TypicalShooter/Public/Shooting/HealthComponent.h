// Health of an actor with an ability system and UHealthAttributeSet: sets the starting health, counts depletions and
// either refills the health (immortal player, for now) or stays depleted (enemies die on the depleted event).

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

class UAbilitySystemComponent;
struct FOnAttributeChangeData;

DECLARE_MULTICAST_DELEGATE(FHealthDepletedNative);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHealthDepletedDynamic);

UCLASS(ClassGroup = (Shooting), meta = (BlueprintSpawnableComponent))
class TYPICALSHOOTER_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	/** Health at start (and after every refill). Set on the server at BeginPlay. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", meta = (ClampMin = 1))
	float MaxHealth = 100.f;

	/** true: at zero health refill to MaxHealth on the next tick; false: stay depleted. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	bool bRefillWhenDepleted = false;

	/** Server: health reached zero (once per depletion). */
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FHealthDepletedDynamic OnDepleted;

	FHealthDepletedNative& GetDepletedDelegate() { return DepletedDelegate; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealth() const;

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealth() const;

	/** Health reached zero and was not refilled yet (server). */
	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDepleted() const { return bDepleted; }

	/** How many times health reached zero (server). */
	UFUNCTION(BlueprintPure, Category = "Health")
	int32 GetDefeats() const { return Defeats; }

protected:
	virtual void BeginPlay() override;

private:
	void OnHealthChanged(const FOnAttributeChangeData& Data);
	void Deplete();
	void Refill();

	FHealthDepletedNative DepletedDelegate;

	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> Asc;

	bool bDepleted = false;
	int32 Defeats = 0;
};
