#include "TypicalShooterPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "InteractionComponent.h"
#include "ItemsContainer.h"
#include "TypicalShooter.h"
#include "TypicalShooterCameraManager.h"
#include "TypicalShooterCharacter.h"

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

	if (!IsLocalPlayerController() || !PrimaryInteractAction)
	{
		return;
	}

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	EnhancedInputComponent->BindAction(PrimaryInteractAction, ETriggerEvent::Started, this, &ATypicalShooterPlayerController::PrimaryInteract);
}

void ATypicalShooterPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// Close the container UI when it was destroyed or the player walked away from it.
	if (bContainerOpened && !IsInInteractionRange(OpenedContainer.Get()))
	{
		CloseItemsContainer();
	}
}

void ATypicalShooterPlayerController::PrimaryInteract_Implementation()
{
	if (bContainerOpened)
	{
		CloseItemsContainer();
		return;
	}

	AActor* Target = FindInteractionTarget();
	if (!Target)
	{
		UE_LOG(LogTypicalShooter, Verbose, TEXT("PrimaryInteract: nothing to interact with"));
		return;
	}

	IPlayerInteractive::Execute_Interact(Target, IPlayerInteractive::Execute_GetInteractionType(Target));

	if (AItemsContainer* Container = Cast<AItemsContainer>(Target))
	{
		OpenedContainer = Container;
		bContainerOpened = true;
		UE_LOG(LogTypicalShooter, Log, TEXT("Opened items container %s"), *Container->GetName());
		OnItemsContainerOpened(Container);
	}
}

void ATypicalShooterPlayerController::CloseItemsContainer()
{
	if (!bContainerOpened)
	{
		return;
	}

	bContainerOpened = false;
	OpenedContainer.Reset();
	UE_LOG(LogTypicalShooter, Log, TEXT("Closed items container"));
	OnItemsContainerClosed();
}

AActor* ATypicalShooterPlayerController::FindInteractionTarget() const
{
	const ATypicalShooterCharacter* ShooterCharacter = Cast<ATypicalShooterCharacter>(GetPawn());
	UInteractionComponent* Interaction = ShooterCharacter ? ShooterCharacter->GetInteractionComponent() : nullptr;
	if (!Interaction)
	{
		return nullptr;
	}

	TArray<AActor*> Candidates;
	Interaction->GetOverlapActors(Candidates);

	const FVector PawnLocation = ShooterCharacter->GetActorLocation();
	AActor* Best = nullptr;
	double BestDistSq = TNumericLimits<double>::Max();
	for (AActor* Candidate : Candidates)
	{
		if (!UInteractionComponent::GetInteractionAPI(Candidate))
		{
			continue;
		}

		const double DistSq = FVector::DistSquared(PawnLocation, Candidate->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Best = Candidate;
		}
	}
	return Best;
}

bool ATypicalShooterPlayerController::IsInInteractionRange(const AActor* Actor) const
{
	if (!IsValid(Actor))
	{
		return false;
	}

	const ATypicalShooterCharacter* ShooterCharacter = Cast<ATypicalShooterCharacter>(GetPawn());
	UInteractionComponent* Interaction = ShooterCharacter ? ShooterCharacter->GetInteractionComponent() : nullptr;
	if (!Interaction)
	{
		return false;
	}

	TArray<AActor*> Overlaps;
	Interaction->GetOverlapActors(Overlaps);
	return Overlaps.Contains(Actor);
}
