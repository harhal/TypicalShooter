#include "TypicalShooterPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "InventoryComponent.h"
#include "InventoryItem.h"
#include "UI/InventoryUISubsystem.h"
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

	if (IsLocalPlayerController())
	{
		// Start in gameplay: focus the game viewport, no cursor.
		ApplyInputMode(false);
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (!Subsystem)
	{
		UE_LOG(LogTypicalShooter, Verbose, TEXT("%s: no Enhanced Input subsystem (not a local player?), mapping contexts not added"), *GetName());
		return;
	}

	for (const TObjectPtr<UInputMappingContext>& Imc : DefaultIMCs)
	{
		Subsystem->AddMappingContext(Imc, 0);
		UE_LOG(LogTypicalShooter, Verbose, TEXT("%s: added mapping context %s"), *GetName(), *GetNameSafe(Imc));
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
	if (PrimaryInteractAction)
	{
		EnhancedInputComponent->BindAction(PrimaryInteractAction, ETriggerEvent::Started, this, &ATypicalShooterPlayerController::PrimaryInteract);
	}
	if (ToggleInventoryAction)
	{
		EnhancedInputComponent->BindAction(ToggleInventoryAction, ETriggerEvent::Started, this, &ATypicalShooterPlayerController::ToggleInventory);
	}
}

void ATypicalShooterPlayerController::ApplyInputMode(bool bUIVisible)
{
	if (bUIVisible)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		SetInputMode(InputMode);
	}
	else
	{
		SetInputMode(FInputModeGameOnly());
	}
	SetShowMouseCursor(bUIVisible);
}

void ATypicalShooterPlayerController::ToggleInventory()
{
	if (bInventoryOpened)
	{
		CloseInventory();
	}
	else
	{
		OpenInventory();
	}
}

void ATypicalShooterPlayerController::OpenInventory()
{
	if (bInventoryOpened)
	{
		return;
	}

	bInventoryOpened = true;
	ApplyInputMode(true);
	UE_LOG(LogTypicalShooter, Log, TEXT("Opened inventory"));
	OnInventoryOpened();
}

void ATypicalShooterPlayerController::CloseInventory()
{
	if (!bInventoryOpened)
	{
		return;
	}

	CloseItemsContainer();

	bInventoryOpened = false;
	ApplyInputMode(false);
	UE_LOG(LogTypicalShooter, Log, TEXT("Closed inventory"));
	OnInventoryClosed();
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
		// The container was opened together with the inventory screen: close both.
		CloseInventory();
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
		OpenInventory();
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

void ATypicalShooterPlayerController::RequestTransferItem(UInventoryComponent* Source, UInventoryItem* Item, UInventoryComponent* Target, const FHhAxialCoord& Location)
{
	// On the server (standalone / listen host) this runs immediately.
	ServerTransferItem(Source, Item, Target, Location);
}

void ATypicalShooterPlayerController::ServerTransferItem_Implementation(UInventoryComponent* Source, UInventoryItem* Item, UInventoryComponent* Target, FHhAxialCoord Location)
{
	if (!CanAccessInventory(Source) || !CanAccessInventory(Target))
	{
		UE_LOG(LogTypicalShooter, Warning, TEXT("%s: rejected item transfer %s -> %s (inventory not accessible)"),
			*GetName(), *GetNameSafe(Source ? Source->GetOwner() : nullptr), *GetNameSafe(Target ? Target->GetOwner() : nullptr));
		return;
	}

	if (!Source->TransferItem(Item, Target, Location))
	{
		UE_LOG(LogTypicalShooter, Verbose, TEXT("%s: item transfer %s -> %s refused by the inventory"),
			*GetName(), *GetNameSafe(Source->GetOwner()), *GetNameSafe(Target->GetOwner()));
	}
}

UInventoryComponent* ATypicalShooterPlayerController::GetPawnInventory() const
{
	const APawn* ControlledPawn = GetPawn();
	return ControlledPawn ? ControlledPawn->FindComponentByClass<UInventoryComponent>() : nullptr;
}

void ATypicalShooterPlayerController::TakeSelectedItem()
{
	const AItemsContainer* Container = GetOpenedItemsContainer();
	const ULocalPlayer* LocalPlayer = GetLocalPlayer();
	const UInventoryUISubsystem* UISubsystem = LocalPlayer ? LocalPlayer->GetSubsystem<UInventoryUISubsystem>() : nullptr;
	UInventoryItem* Selected = UISubsystem ? UISubsystem->GetSelectedItem() : nullptr;
	if (!Container || !Selected || Selected->GetOuter() != Container->InventoryComponent)
	{
		return;
	}
	ServerTakeItems(Container->InventoryComponent, { Selected });
}

void ATypicalShooterPlayerController::TakeAllFromOpenedContainer()
{
	if (const AItemsContainer* Container = GetOpenedItemsContainer())
	{
		ServerTakeItems(Container->InventoryComponent, {});
	}
}

void ATypicalShooterPlayerController::ServerTakeItems_Implementation(UInventoryComponent* Source, const TArray<UInventoryItem*>& Items)
{
	UInventoryComponent* Target = GetPawnInventory();
	if (!Target || !CanAccessInventory(Source))
	{
		UE_LOG(LogTypicalShooter, Warning, TEXT("%s: rejected taking items from %s"), *GetName(), *GetNameSafe(Source ? Source->GetOwner() : nullptr));
		return;
	}

	TArray<UInventoryItem*> ToTake = Items;
	if (ToTake.IsEmpty())
	{
		for (const TPair<UInventoryItem*, FHhAxialCoord>& Placed : Source->GetPlacedItems())
		{
			ToTake.Add(Placed.Key);
		}
	}

	for (UInventoryItem* Item : ToTake)
	{
		if (!Source->TransferItemToFreeLocation(Item, Target))
		{
			break; // the pawn's inventory is full (or the item is gone)
		}
	}
}

bool ATypicalShooterPlayerController::CanAccessInventory(const UInventoryComponent* Inventory) const
{
	const APawn* ControlledPawn = GetPawn();
	if (!Inventory || !ControlledPawn)
	{
		return false;
	}

	const AActor* InventoryOwner = Inventory->GetOwner();
	if (InventoryOwner == ControlledPawn)
	{
		return true;
	}

	const AItemsContainer* Container = Cast<AItemsContainer>(InventoryOwner);
	return Container && Container->CanBeAccessedBy(ControlledPawn);
}
