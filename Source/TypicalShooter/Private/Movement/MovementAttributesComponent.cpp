#include "Movement/MovementAttributesComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Movement/MovementAttributeSet.h"

UMovementAttributesComponent::UMovementAttributesComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMovementAttributesComponent::BeginPlay()
{
	Super::BeginPlay();
	UAbilitySystemComponent* AscPtr = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
	const UCharacterMovementComponent* Movement = GetMovement();
	if (!AscPtr || !Movement || !AscPtr->GetSet<UMovementAttributeSet>())
	{
		return;
	}
	Asc = AscPtr;
	JumpVelocityHandle = AscPtr->GetGameplayAttributeValueChangeDelegate(UMovementAttributeSet::GetJumpVelocityAttribute())
		.AddUObject(this, &ThisClass::HandleAttributeChanged);
	AirControlHandle = AscPtr->GetGameplayAttributeValueChangeDelegate(UMovementAttributeSet::GetAirControlAttribute())
		.AddUObject(this, &ThisClass::HandleAttributeChanged);

	// Clients wait for the replicated values (OnRep fires the change delegate) instead of applying zeros.
	if (GetOwner()->HasAuthority())
	{
		SeedFromMovement(*AscPtr, *Movement);
	}
}

void UMovementAttributesComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UAbilitySystemComponent* AscPtr = Asc.Get())
	{
		AscPtr->GetGameplayAttributeValueChangeDelegate(UMovementAttributeSet::GetJumpVelocityAttribute()).Remove(JumpVelocityHandle);
		AscPtr->GetGameplayAttributeValueChangeDelegate(UMovementAttributeSet::GetAirControlAttribute()).Remove(AirControlHandle);
	}
	Super::EndPlay(EndPlayReason);
}

void UMovementAttributesComponent::SeedFromMovement(UAbilitySystemComponent& InAsc, const UCharacterMovementComponent& Movement) const
{
	InAsc.SetNumericAttributeBase(UMovementAttributeSet::GetJumpVelocityAttribute(), Movement.JumpZVelocity);
	InAsc.SetNumericAttributeBase(UMovementAttributeSet::GetAirControlAttribute(), Movement.AirControl);
}

void UMovementAttributesComponent::ApplyToMovement(const FGameplayAttribute& Attribute, const float Value) const
{
	UCharacterMovementComponent* Movement = GetMovement();
	if (!Movement)
	{
		return;
	}
	if (Attribute == UMovementAttributeSet::GetJumpVelocityAttribute())
	{
		Movement->JumpZVelocity = Value;
	}
	else if (Attribute == UMovementAttributeSet::GetAirControlAttribute())
	{
		Movement->AirControl = Value;
	}
}

void UMovementAttributesComponent::HandleAttributeChanged(const FOnAttributeChangeData& Data) const
{
	ApplyToMovement(Data.Attribute, Data.NewValue);
}

UCharacterMovementComponent* UMovementAttributesComponent::GetMovement() const
{
	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	return Character ? Character->GetCharacterMovement() : nullptr;
}
