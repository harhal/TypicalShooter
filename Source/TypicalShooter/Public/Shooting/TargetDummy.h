// Training target: GAS health with a floating "health / max" label; never dies (refills when depleted).

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Actor.h"
#include "TargetDummy.generated.h"

class UCapsuleComponent;
class UGasDamageReceiverComponent;
class UHealthAttributeSet;
class UStaticMeshComponent;
class UTextRenderComponent;
struct FOnAttributeChangeData;

UCLASS()
class TYPICALSHOOTER_API ATargetDummy : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ATargetDummy();

	/** Health at start and after every refill. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target Dummy", meta = (ClampMin = 1))
	float MaxHealth = 100.f;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintPure, Category = "Target Dummy")
	float GetHealth() const;

	/** How many times health reached zero (server). */
	UFUNCTION(BlueprintPure, Category = "Target Dummy")
	int32 GetDefeats() const { return Defeats; }

	/** Sum of all damage taken (server). */
	UFUNCTION(BlueprintPure, Category = "Target Dummy")
	float GetTotalDamageTaken() const { return TotalDamageTaken; }

	UTextRenderComponent* GetHealthText() const { return HealthText; }

protected:
	virtual void BeginPlay() override;

private:
	void OnHealthChanged(const FOnAttributeChangeData& Data);
	void Refill();
	void UpdateHealthText() const;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UCapsuleComponent> Collision;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UTextRenderComponent> HealthText;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UAbilitySystemComponent> AbilitySystem;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UGasDamageReceiverComponent> DamageReceiver;

	UPROPERTY()
	TObjectPtr<UHealthAttributeSet> HealthSet;

	UPROPERTY(VisibleInstanceOnly, Category = "Target Dummy")
	int32 Defeats = 0;

	UPROPERTY(VisibleInstanceOnly, Category = "Target Dummy")
	float TotalDamageTaken = 0.f;
};
