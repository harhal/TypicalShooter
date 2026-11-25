#include "TypicalShooterPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
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

	CreateInventoryInput();
	Subsystem->AddMappingContext(GamepadIMC, 0);
}

namespace TypicalShooterPlayerControllerPrivate
{
	enum class EAxisMapping : uint8
	{
		AsIs,
		Negate,
		/** Button value onto the Y axis. */
		Swizzle,
		SwizzleNegate,
	};

	void MapKey(UInputMappingContext* Imc, const UInputAction* Action, const FKey& Key, const EAxisMapping Mapping = EAxisMapping::AsIs)
	{
		if (!Action)
		{
			return;
		}
		FEnhancedActionKeyMapping& KeyMapping = Imc->MapKey(Action, Key);
		if (Mapping == EAxisMapping::Swizzle || Mapping == EAxisMapping::SwizzleNegate)
		{
			KeyMapping.Modifiers.Add(NewObject<UInputModifierSwizzleAxis>(Imc));
		}
		if (Mapping == EAxisMapping::Negate || Mapping == EAxisMapping::SwizzleNegate)
		{
			KeyMapping.Modifiers.Add(NewObject<UInputModifierNegate>(Imc));
		}
	}

	UInputAction* MakeAction(UObject* Outer, const FName Name, const EInputActionValueType ValueType = EInputActionValueType::Boolean)
	{
		UInputAction* Action = NewObject<UInputAction>(Outer, Name, RF_Transient);
		Action->ValueType = ValueType;
		return Action;
	}

	constexpr float NavigationThreshold = 0.5f;
	constexpr double NavigationFirstRepeatDelay = 0.35;
	constexpr double NavigationRepeatInterval = 0.12;
}

void ATypicalShooterPlayerController::CreateInventoryInput()
{
	using namespace TypicalShooterPlayerControllerPrivate;
	if (InventoryIMC)
	{
		return;
	}

	// Gameplay (always on): View opens the inventory, X interacts. Keyboard keys live in IMC_Default.
	GamepadIMC = NewObject<UInputMappingContext>(this, TEXT("IMC_Gamepad_Generated"), RF_Transient);
	MapKey(GamepadIMC, ToggleInventoryAction, EKeys::Gamepad_Special_Left);
	MapKey(GamepadIMC, PrimaryInteractAction, EKeys::Gamepad_FaceButton_Left);

	InventoryNavigateAction = MakeAction(this, TEXT("IA_InventoryNavigate_Generated"), EInputActionValueType::Axis2D);
	InventoryConfirmAction = MakeAction(this, TEXT("IA_InventoryConfirm_Generated"));
	InventoryCancelAction = MakeAction(this, TEXT("IA_InventoryCancel_Generated"));
	InventorySwitchGridAction = MakeAction(this, TEXT("IA_InventorySwitchGrid_Generated"), EInputActionValueType::Axis1D);
	InventoryTakeAction = MakeAction(this, TEXT("IA_InventoryTake_Generated"));
	InventoryTakeAllAction = MakeAction(this, TEXT("IA_InventoryTakeAll_Generated"));
	InventoryRotateAction = MakeAction(this, TEXT("IA_InventoryRotate_Generated"));

	// Inventory screen (higher priority while open; actions consume their keys).
	InventoryIMC = NewObject<UInputMappingContext>(this, TEXT("IMC_Inventory_Generated"), RF_Transient);
	MapKey(InventoryIMC, InventoryNavigateAction, EKeys::Gamepad_DPad_Right);
	MapKey(InventoryIMC, InventoryNavigateAction, EKeys::Gamepad_DPad_Left, EAxisMapping::Negate);
	MapKey(InventoryIMC, InventoryNavigateAction, EKeys::Gamepad_DPad_Up, EAxisMapping::Swizzle);
	MapKey(InventoryIMC, InventoryNavigateAction, EKeys::Gamepad_DPad_Down, EAxisMapping::SwizzleNegate);
	MapKey(InventoryIMC, InventoryNavigateAction, EKeys::Right);
	MapKey(InventoryIMC, InventoryNavigateAction, EKeys::Left, EAxisMapping::Negate);
	MapKey(InventoryIMC, InventoryNavigateAction, EKeys::Up, EAxisMapping::Swizzle);
	MapKey(InventoryIMC, InventoryNavigateAction, EKeys::Down, EAxisMapping::SwizzleNegate);
	MapKey(InventoryIMC, InventoryNavigateAction, EKeys::Gamepad_Left2D);

	MapKey(InventoryIMC, InventoryConfirmAction, EKeys::Gamepad_FaceButton_Bottom);
	MapKey(InventoryIMC, InventoryConfirmAction, EKeys::Enter);
	MapKey(InventoryIMC, InventoryCancelAction, EKeys::Gamepad_FaceButton_Right);
	MapKey(InventoryIMC, InventorySwitchGridAction, EKeys::Gamepad_RightShoulder);
	MapKey(InventoryIMC, InventorySwitchGridAction, EKeys::Gamepad_LeftShoulder, EAxisMapping::Negate);
	MapKey(InventoryIMC, InventoryTakeAction, EKeys::Gamepad_FaceButton_Left);
	MapKey(InventoryIMC, InventoryTakeAction, EKeys::R);
	MapKey(InventoryIMC, InventoryTakeAllAction, EKeys::Gamepad_FaceButton_Top);
	MapKey(InventoryIMC, InventoryTakeAllAction, EKeys::F);
	MapKey(InventoryIMC, InventoryRotateAction, EKeys::Gamepad_RightTrigger);
	MapKey(InventoryIMC, InventoryRotateAction, EKeys::Q);
}

UInventoryUISubsystem* ATypicalShooterPlayerController::GetInventoryUI() const
{
	const ULocalPlayer* LocalPlayer = GetLocalPlayer();
	return LocalPlayer ? LocalPlayer->GetSubsystem<UInventoryUISubsystem>() : nullptr;
}

void ATypicalShooterPlayerController::UpdateNavigableContainers()
{
	UInventoryUISubsystem* UI = GetInventoryUI();
	if (!UI)
	{
		return;
	}

	TArray<UObject*> Containers;
	if (bInventoryOpened)
	{
		if (UInventoryComponent* PawnInventory = GetPawnInventory())
		{
			Containers.Add(PawnInventory);
		}
		if (const AItemsContainer* Container = GetOpenedItemsContainer(); Container && bContainerOpened)
		{
			Containers.Add(Container->InventoryComponent);
		}
	}
	UI->SetNavigableContainers(Containers);
}

void ATypicalShooterPlayerController::HandleInventoryNavigate(const FInputActionValue& Value)
{
	using namespace TypicalShooterPlayerControllerPrivate;

	const FVector2D Direction = Value.Get<FVector2D>();
	UInventoryUISubsystem* UI = GetInventoryUI();
	if (!UI || Direction.Size() < NavigationThreshold)
	{
		bNavigationHeld = false;
		return;
	}

	// One step per press, auto-repeat while held; a new direction steps at once.
	const int32 Sector = FMath::RoundToInt(FMath::Atan2(Direction.Y, Direction.X) / (UE_DOUBLE_PI / 4.0)) & 7;
	const bool bSameDirection = bNavigationHeld && Sector == NavigationSector;
	const double Now = FPlatformTime::Seconds();
	if (bSameDirection && Now < NextNavigationRepeatTime)
	{
		return;
	}

	UI->MoveCursor(Direction);
	NextNavigationRepeatTime = Now + (bSameDirection ? NavigationRepeatInterval : NavigationFirstRepeatDelay);
	bNavigationHeld = true;
	NavigationSector = Sector;
}

void ATypicalShooterPlayerController::HandleInventoryNavigateReleased()
{
	bNavigationHeld = false;
}

void ATypicalShooterPlayerController::HandleInventoryConfirm()
{
	if (UInventoryUISubsystem* UI = GetInventoryUI())
	{
		UI->ConfirmAtCursor();
	}
}

void ATypicalShooterPlayerController::HandleInventoryCancel()
{
	UInventoryUISubsystem* UI = GetInventoryUI();
	if (!UI || !UI->CancelSelection())
	{
		CloseInventory();
	}
}

void ATypicalShooterPlayerController::HandleInventorySwitchGrid(const FInputActionValue& Value)
{
	const float Step = Value.Get<float>();
	if (UInventoryUISubsystem* UI = GetInventoryUI(); UI && !FMath::IsNearlyZero(Step))
	{
		UI->SwitchCursorContainer(Step > 0.f ? 1 : -1);
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

	CreateInventoryInput();
	EnhancedInputComponent->BindAction(InventoryNavigateAction, ETriggerEvent::Triggered, this, &ATypicalShooterPlayerController::HandleInventoryNavigate);
	EnhancedInputComponent->BindAction(InventoryNavigateAction, ETriggerEvent::Completed, this, &ATypicalShooterPlayerController::HandleInventoryNavigateReleased);
	EnhancedInputComponent->BindAction(InventoryConfirmAction, ETriggerEvent::Started, this, &ATypicalShooterPlayerController::HandleInventoryConfirm);
	EnhancedInputComponent->BindAction(InventoryCancelAction, ETriggerEvent::Started, this, &ATypicalShooterPlayerController::HandleInventoryCancel);
	EnhancedInputComponent->BindAction(InventorySwitchGridAction, ETriggerEvent::Started, this, &ATypicalShooterPlayerController::HandleInventorySwitchGrid);
	EnhancedInputComponent->BindAction(InventoryTakeAction, ETriggerEvent::Started, this, &ATypicalShooterPlayerController::TakeFocusedItem);
	EnhancedInputComponent->BindAction(InventoryTakeAllAction, ETriggerEvent::Started, this, &ATypicalShooterPlayerController::TakeAllFromOpenedContainer);
	EnhancedInputComponent->BindAction(InventoryRotateAction, ETriggerEvent::Started, this, &ATypicalShooterPlayerController::RotateFocusedItem);
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
	if (UEnhancedInputLocalPlayerSubsystem* Input = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()); Input && InventoryIMC)
	{
		Input->AddMappingContext(InventoryIMC, InventoryIMCPriority);
	}
	UpdateNavigableContainers();
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
	if (UEnhancedInputLocalPlayerSubsystem* Input = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()); Input && InventoryIMC)
	{
		Input->RemoveMappingContext(InventoryIMC);
	}
	bNavigationHeld = false;
	UpdateNavigableContainers();
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
		UpdateNavigableContainers();
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
	UpdateNavigableContainers();
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
	const UInventoryUISubsystem* UI = GetInventoryUI();
	TakeItem(UI ? UI->GetSelectedItem() : nullptr);
}

void ATypicalShooterPlayerController::TakeFocusedItem()
{
	const UInventoryUISubsystem* UI = GetInventoryUI();
	TakeItem(UI ? UI->GetFocusedItem() : nullptr);
}

void ATypicalShooterPlayerController::TakeItem(UInventoryItem* Item)
{
	const AItemsContainer* Container = GetOpenedItemsContainer();
	if (!Container || !Item || Item->GetOuter() != Container->InventoryComponent)
	{
		return;
	}
	ServerTakeItems(Container->InventoryComponent, { Item });
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

	// One layout event per inventory for the whole take.
	UInventoryComponent::FScopedLayoutBatch SourceBatch(Source);
	UInventoryComponent::FScopedLayoutBatch TargetBatch(Target);
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

void ATypicalShooterPlayerController::RequestRotateItem(UInventoryComponent* Inventory, UInventoryItem* Item, const int32 Steps)
{
	// On the server (standalone / listen host) this runs immediately.
	ServerRotateItem(Inventory, Item, Steps);
}

void ATypicalShooterPlayerController::ServerRotateItem_Implementation(UInventoryComponent* Inventory, UInventoryItem* Item, const int32 Steps)
{
	if (!CanAccessInventory(Inventory))
	{
		UE_LOG(LogTypicalShooter, Warning, TEXT("%s: rejected item rotation in %s (inventory not accessible)"),
			*GetName(), *GetNameSafe(Inventory ? Inventory->GetOwner() : nullptr));
		return;
	}
	Inventory->RotateItem(Item, Steps);
}

void ATypicalShooterPlayerController::RotateFocusedItem()
{
	const UInventoryUISubsystem* UI = GetInventoryUI();
	UInventoryItem* Item = UI ? UI->GetFocusedItem() : nullptr;
	Item = Item ? Item : (UI ? UI->GetHoveredItem() : nullptr);
	if (UInventoryComponent* Inventory = Item ? Item->GetTypedOuter<UInventoryComponent>() : nullptr)
	{
		RequestRotateItem(Inventory, Item);
	}
}
