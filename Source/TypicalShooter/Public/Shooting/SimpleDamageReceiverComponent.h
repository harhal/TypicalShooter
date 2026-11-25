// Damage receiver without GAS: a plain durability pool (breakable walls, crates).

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Shooting/ShotDamageReceiver.h"
#include "SimpleDamageReceiverComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDurabilityDepleted, AActor*, Instigator);

UCLASS(ClassGroup = (Shooting), meta = (BlueprintSpawnableComponent))
class TYPICALSHOOTER_API USimpleDamageReceiverComponent : public UActorComponent, public IShotDamageReceiver
{
	GENERATED_BODY()

public:
	USimpleDamageReceiverComponent();

	/** Durability at start. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage", meta = (ClampMin = 0))
	float MaxDurability = 100.f;

	/** Destroy the owning actor when durability reaches zero. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	bool bDestroyOwnerOnDepleted = false;

	/** Fired once when durability reaches zero (server). */
	UPROPERTY(BlueprintAssignable, Category = "Damage")
	FOnDurabilityDepleted OnDepleted;

	virtual void InitializeComponent() override;
	virtual void ReceiveShotDamage(const FShotDamage& Damage, const FHitResult& Hit) override;

	UFUNCTION(BlueprintPure, Category = "Damage")
	float GetDurability() const { return Durability; }

	UFUNCTION(BlueprintPure, Category = "Damage")
	bool IsDepleted() const { return Durability <= 0.f; }

	/** Native counterpart of OnDepleted. */
	FSimpleMulticastDelegate& GetDepletedDelegate() { return DepletedDelegate; }

private:
	FSimpleMulticastDelegate DepletedDelegate;

	UPROPERTY(VisibleInstanceOnly, Category = "Damage")
	float Durability = 0.f;
};
