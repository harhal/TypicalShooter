#include "TypicalShooterPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "TypicalShooterCameraManager.h"

ATypicalShooterPlayerController::ATypicalShooterPlayerController()
{
	PlayerCameraManagerClass = ATypicalShooterCameraManager::StaticClass();
}

void ATypicalShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (!Subsystem)
	{
		return;
	}
	
	for (const TObjectPtr<UInputMappingContext>& Imc : DefaultIMCs)
	{
		Subsystem->AddMappingContext(Imc, 0);
	}
}

void ATypicalShooterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!IsLocalPlayerController())
	{
		return;
	}
		
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	EnhancedInputComponent->BindAction(PrimaryInteractAction, ETriggerEvent::Started, this, &ATypicalShooterPlayerController::PrimaryInteract);	
}

void ATypicalShooterPlayerController::PrimaryInteract_Implementation()
{
}