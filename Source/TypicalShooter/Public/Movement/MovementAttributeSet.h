// Character movement parameters as GAS attributes, so item effects can change them (power grid R6c, R13).
// UMovementAttributesComponent mirrors them into the UCharacterMovementComponent.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "MovementAttributeSet.generated.h"

UCLASS()
class TYPICALSHOOTER_API UMovementAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	ATTRIBUTE_ACCESSORS_BASIC(UMovementAttributeSet, JumpVelocity)
	ATTRIBUTE_ACCESSORS_BASIC(UMovementAttributeSet, AirControl)

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	/** Jump power: vertical launch speed of a ground jump, cm/s (UCharacterMovementComponent::JumpZVelocity). */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_JumpVelocity, Category = "Movement")
	FGameplayAttributeData JumpVelocity;

	/** Lateral control while falling (UCharacterMovementComponent::AirControl). */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AirControl, Category = "Movement")
	FGameplayAttributeData AirControl;

protected:
	// Clients mirror the values into their movement component (prediction uses them too).
	UFUNCTION() void OnRep_JumpVelocity(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_AirControl(const FGameplayAttributeData& OldValue);
};
