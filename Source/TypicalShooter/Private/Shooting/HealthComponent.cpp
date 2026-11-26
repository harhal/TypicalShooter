#include "Shooting/HealthComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Engine/World.h"
#include "Shooting/HealthAttributeSet.h"
#include "TimerManager.h"
#include "TypicalShooter.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

float UHealthComponent::GetHealth() const
{
	return Asc ? Asc->GetNumericAttribute(UHealthAttributeSet::GetHealthAttribute()) : 0.f;
}

float UHealthComponent::GetMaxHealth() const
{
	return Asc ? Asc->GetNumericAttribute(UHealthAttributeSet::GetMaxHealthAttribute()) : 0.f;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	Asc = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
	if (!Asc || !Asc->HasAttributeSetForAttribute(UHealthAttributeSet::GetHealthAttribute()))
	{
		UE_LOG(LogTypicalShooter, Warning, TEXT("%s: health component needs an ASC with a health attribute set"), *GetNameSafe(GetOwner()));
		return;
	}

	Asc->GetGameplayAttributeValueChangeDelegate(UHealthAttributeSet::GetHealthAttribute())
		.AddUObject(this, &UHealthComponent::OnHealthChanged);
	if (GetOwner()->HasAuthority())
	{
		Asc->SetNumericAttributeBase(UHealthAttributeSet::GetMaxHealthAttribute(), MaxHealth);
		Asc->SetNumericAttributeBase(UHealthAttributeSet::GetHealthAttribute(), MaxHealth);
	}
}

void UHealthComponent::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	const float Lost = Data.OldValue - Data.NewValue;
	if (!GetOwner()->HasAuthority() || Lost <= 0.f)
	{
		return;
	}

	UE_LOG(LogTypicalShooter, Verbose, TEXT("%s took %.1f damage, health %.1f"), *GetNameSafe(GetOwner()), Lost, Data.NewValue);
	if (Data.NewValue <= 0.f && !bDepleted)
	{
		Deplete();
	}
}

void UHealthComponent::Deplete()
{
	bDepleted = true;
	++Defeats;
	UE_LOG(LogTypicalShooter, Log, TEXT("%s: health depleted (%d times)%s"), *GetNameSafe(GetOwner()), Defeats,
		bRefillWhenDepleted ? TEXT(", refilling") : TEXT(""));
	if (bRefillWhenDepleted)
	{
		// Refill outside of the effect execution that changed health.
		GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &UHealthComponent::Refill));
	}
	DepletedDelegate.Broadcast();
	OnDepleted.Broadcast();
}

void UHealthComponent::Refill()
{
	bDepleted = false;
	Asc->SetNumericAttributeBase(UHealthAttributeSet::GetHealthAttribute(), MaxHealth);
}
