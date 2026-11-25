// Keeps the owner character's movement component in sync with its UMovementAttributeSet (power grid R6c).
// The server seeds the attributes from the movement component's configured values at BeginPlay; from then on every
// attribute change (effects, replication to clients) is written back into the movement component.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MovementAttributesComponent.generated.h"

class UAbilitySystemComponent;
class UCharacterMovementComponent;
struct FGameplayAttribute;
struct FOnAttributeChangeData;

UCLASS(ClassGroup = (Movement), meta = (BlueprintSpawnableComponent))
class TYPICALSHOOTER_API UMovementAttributesComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMovementAttributesComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

private:
	void SeedFromMovement(UAbilitySystemComponent& Asc, const UCharacterMovementComponent& Movement) const;
	/** Writes one attribute into the movement component (only the changed one: on clients the other may not be replicated yet). */
	void ApplyToMovement(const FGameplayAttribute& Attribute, float Value) const;
	void HandleAttributeChanged(const FOnAttributeChangeData& Data) const;
	UCharacterMovementComponent* GetMovement() const;

	TWeakObjectPtr<UAbilitySystemComponent> Asc;
	FDelegateHandle JumpVelocityHandle;
	FDelegateHandle AirControlHandle;
};
