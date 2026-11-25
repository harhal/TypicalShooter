#include "Shooting/AttackInputComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Pawn.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "TypicalShooterPlayerController.h"

namespace AttackInputComponentPrivate
{
	UInputAction* MakeAction(UObject* Outer, const FName Name)
	{
		UInputAction* Action = NewObject<UInputAction>(Outer, Name, RF_Transient);
		Action->ValueType = EInputActionValueType::Boolean;
		return Action;
	}
}

UAttackInputComponent::UAttackInputComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAttackInputComponent::BindInput(UEnhancedInputComponent& Input)
{
	CreateInput();
	const auto Bind = [this, &Input](const UInputAction* Action, const EAttackInputID Id)
	{
		Input.BindAction(Action, ETriggerEvent::Started, this, &UAttackInputComponent::HandlePressed, Id);
		Input.BindAction(Action, ETriggerEvent::Completed, this, &UAttackInputComponent::HandleReleased, Id);
	};
	Bind(PrimaryAction, EAttackInputID::PrimaryAttack);
	Bind(AlternateAction, EAttackInputID::AlternateAttack);
	Bind(ReloadAction, EAttackInputID::Reload);
	AddMappingContext();
}

void UAttackInputComponent::CreateInput()
{
	using namespace AttackInputComponentPrivate;
	if (MappingContext)
	{
		return;
	}
	PrimaryAction = MakeAction(this, TEXT("IA_PrimaryAttack_Generated"));
	AlternateAction = MakeAction(this, TEXT("IA_AlternateAttack_Generated"));
	ReloadAction = MakeAction(this, TEXT("IA_Reload_Generated"));

	MappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_Attack_Generated"), RF_Transient);
	MappingContext->MapKey(PrimaryAction, EKeys::LeftMouseButton);
	MappingContext->MapKey(PrimaryAction, EKeys::Gamepad_RightTrigger);
	MappingContext->MapKey(AlternateAction, EKeys::RightMouseButton);
	MappingContext->MapKey(AlternateAction, EKeys::Gamepad_LeftTrigger);
	MappingContext->MapKey(ReloadAction, EKeys::R);
	MappingContext->MapKey(ReloadAction, EKeys::Gamepad_FaceButton_Right);
}

void UAttackInputComponent::AddMappingContext() const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	const APlayerController* Controller = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	UEnhancedInputLocalPlayerSubsystem* Subsystem = Controller
		? ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Controller->GetLocalPlayer()) : nullptr;
	if (Subsystem)
	{
		Subsystem->AddMappingContext(MappingContext, MappingPriority);
	}
}

void UAttackInputComponent::HandlePressed(const EAttackInputID Id)
{
	UAbilitySystemComponent* Asc = GetAbilitySystem();
	if (Asc && !IsInventoryOpen())
	{
		Asc->AbilityLocalInputPressed(static_cast<int32>(Id));
	}
}

void UAttackInputComponent::HandleReleased(const EAttackInputID Id)
{
	if (UAbilitySystemComponent* Asc = GetAbilitySystem())
	{
		Asc->AbilityLocalInputReleased(static_cast<int32>(Id));
	}
}

UAbilitySystemComponent* UAttackInputComponent::GetAbilitySystem() const
{
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
}

bool UAttackInputComponent::IsInventoryOpen() const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	const ATypicalShooterPlayerController* Controller = Pawn ? Cast<ATypicalShooterPlayerController>(Pawn->GetController()) : nullptr;
	return Controller && Controller->IsInventoryOpened();
}
