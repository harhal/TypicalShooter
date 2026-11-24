// End-to-end check of the inventory screen / container flow in Play In Editor.
//
// Starts PIE on the currently open map (expects Lvl_FirstPerson with at least one AItemsContainer).
// Keys are fed through UGameViewportClient::InputKey, i.e. the entry point real keyboard input uses
// after Slate, so the Enhanced Input mappings (IMC_Default: Tab, E, W, SpaceBar) and the
// controller / BP wiring are exercised for real.
// Not covered: OS -> Slate focus routing (whether the game viewport actually has keyboard focus).

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Components/Widget.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "InventoryComponent.h"
#include "ItemsContainer.h"
#include "Tests/AutomationEditorCommon.h"
#include "Engine/LocalPlayer.h"
#include "InventoryItem.h"
#include "TypicalShooterPlayerController.h"
#include "UI/InventoryUISubsystem.h"
#include "UI/Slate/InventoryGrid.h"
#include "UI/Widget/InventoryGridWidget.h"
#include "GameFramework/PawnMovementComponent.h"
#include "UObject/UObjectIterator.h"

namespace InventoryUIFlowTest
{
	UWorld* FindPIEWorld()
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::PIE && Context.World())
			{
				return Context.World();
			}
		}
		return nullptr;
	}

	/** Reads a UObject property by name (used for Blueprint-defined widget variables). */
	UObject* GetObjectProperty(const UObject* Object, const FName Name)
	{
		if (!Object)
		{
			return nullptr;
		}
		const FObjectPropertyBase* Property = FindFProperty<FObjectPropertyBase>(Object->GetClass(), Name);
		return Property ? Property->GetObjectPropertyValue_InContainer(Object) : nullptr;
	}

	bool IsWidgetShown(const UObject* Owner, const FName WidgetName)
	{
		const UWidget* Widget = Cast<UWidget>(GetObjectProperty(Owner, WidgetName));
		return Widget && Widget->IsVisible();
	}

	/** The HUD grid bound to the pawn's own inventory (lives inside BP_InventoryGrid). */
	const UInventoryGridWidget* FindPawnInventoryGrid(const UWorld* World)
	{
		for (TObjectIterator<UInventoryGridWidget> It; It; ++It)
		{
			if (It->bShowOwningPawnInventory && It->GetWorld() == World && It->GetSlateGrid().IsValid())
			{
				return *It;
			}
		}
		return nullptr;
	}

	struct FState
	{
		bool bReady = false;
		double WaitStarted = 0.0;
		TWeakObjectPtr<UWorld> World;
		TWeakObjectPtr<ATypicalShooterPlayerController> Controller;
		TWeakObjectPtr<AItemsContainer> Container;
		FVector SavedLocation = FVector::ZeroVector;
		int32 NumContainerItems = 0;

		UObject* GetHUD() const { return GetObjectProperty(Controller.Get(), TEXT("HUD")); }

		/** Tries to resolve PIE objects; true once everything (incl. the BP-created HUD) exists. */
		bool TryResolve()
		{
			World = FindPIEWorld();
			if (!World.IsValid())
			{
				return false;
			}
			Controller = Cast<ATypicalShooterPlayerController>(World->GetFirstPlayerController());
			if (!Controller.IsValid() || !Controller->GetPawn() || !GetHUD())
			{
				return false;
			}
			for (TActorIterator<AItemsContainer> It(World.Get()); It; ++It)
			{
				Container = *It;
				break;
			}
			return Container.IsValid();
		}

		void SendKey(const FKey& Key, const EInputEvent Event) const
		{
			UGameViewportClient* ViewportClient = World.IsValid() ? World->GetGameViewport() : nullptr;
			if (!ViewportClient)
			{
				return;
			}
			const FInputDeviceId Device = IPlatformInputDeviceMapper::Get().GetDefaultInputDevice();
			ViewportClient->InputKey(FInputKeyEventArgs(ViewportClient->Viewport, Device, Key, Event, FPlatformTime::Cycles64()));
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryUIFlowTest, "TypicalShooter.Inventory.UIFlow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FInventoryUIFlowTest::RunTest(const FString& Parameters)
{
	using namespace InventoryUIFlowTest;

	const TSharedRef<FState> State = MakeShared<FState>();

	// Runs Fn after Delay seconds, only if the PIE setup succeeded.
	auto Step = [State](TFunction<void()> Fn, const float Delay = 0.3f)
	{
		ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([State, Fn = MoveTemp(Fn)]()
		{
			if (State->bReady)
			{
				Fn();
			}
		}, Delay));
	};
	// Taps a key: press now, release shortly after.
	auto Tap = [&Step, State](const FKey Key)
	{
		Step([State, Key]() { State->SendKey(Key, IE_Pressed); }, 0.0f);
		Step([State, Key]() { State->SendKey(Key, IE_Released); }, 0.1f);
	};

	// 0. Start PIE and wait (up to 15 s) for the controller, pawn, HUD and a container.
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, State]()
	{
		if (State->WaitStarted == 0.0)
		{
			State->WaitStarted = FPlatformTime::Seconds();
		}
		if (State->TryResolve())
		{
			State->bReady = true;
			return true;
		}
		if (FPlatformTime::Seconds() - State->WaitStarted > 15.0)
		{
			AddError(TEXT("PIE did not provide ATypicalShooterPlayerController + pawn + HUD + AItemsContainer within 15 s."));
			return true;
		}
		return false;
	}));

	// 1. Initial state.
	Step([this, State]()
	{
		ATypicalShooterPlayerController* PC = State->Controller.Get();
		TestFalse(TEXT("Inventory is closed at start"), PC->IsInventoryOpened());
		TestFalse(TEXT("No mouse cursor at start"), PC->bShowMouseCursor);
		TestFalse(TEXT("InventoryScreen hidden at start"), IsWidgetShown(State->GetHUD(), TEXT("InventoryScreen")));
	}, 0.5f);

	// 2. Tab opens and closes the inventory.
	Tap(EKeys::Tab);
	Step([this, State]()
	{
		ATypicalShooterPlayerController* PC = State->Controller.Get();
		TestTrue(TEXT("Tab opens the inventory"), PC->IsInventoryOpened());
		TestTrue(TEXT("Mouse cursor shown with inventory"), PC->bShowMouseCursor);
		TestTrue(TEXT("InventoryScreen shown"), IsWidgetShown(State->GetHUD(), TEXT("InventoryScreen")));
		TestFalse(TEXT("Container grid hidden without a container"), IsWidgetShown(State->GetHUD(), TEXT("BP_ContainerGrid")));
	});
	Tap(EKeys::Tab);
	Step([this, State]()
	{
		ATypicalShooterPlayerController* PC = State->Controller.Get();
		TestFalse(TEXT("Second Tab closes the inventory"), PC->IsInventoryOpened());
		TestFalse(TEXT("Mouse cursor hidden again"), PC->bShowMouseCursor);
		TestFalse(TEXT("InventoryScreen hidden again"), IsWidgetShown(State->GetHUD(), TEXT("InventoryScreen")));
	});

	// 3. Walk up to a container (teleport), let the async interaction overlap update, press E.
	Step([State]()
	{
		APawn* Pawn = State->Controller->GetPawn();
		const FVector Target = State->Container->GetActorLocation() + FVector(-60.f, 0.f, 50.f);
		Pawn->TeleportTo(Target, Pawn->GetActorRotation());
	}, 0.0f);
	Step([]() {}, 0.5f);
	Tap(EKeys::E);
	Step([this, State]()
	{
		ATypicalShooterPlayerController* PC = State->Controller.Get();
		UObject* HUD = State->GetHUD();
		TestTrue(TEXT("E at a container opens the inventory"), PC->IsInventoryOpened());
		TestTrue(TEXT("E at a container opens that container"), PC->GetOpenedItemsContainer() == State->Container.Get());
		TestTrue(TEXT("InventoryScreen shown"), IsWidgetShown(HUD, TEXT("InventoryScreen")));
		TestTrue(TEXT("Container grid shown"), IsWidgetShown(HUD, TEXT("BP_ContainerGrid")));

		const UInventoryGridWidget* Grid = Cast<UInventoryGridWidget>(
			GetObjectProperty(GetObjectProperty(HUD, TEXT("BP_ContainerGrid")), TEXT("InventoryGridWidget_237")));
		const int32 ExpectedItems = State->Container->InventoryComponent->GetNumItems();
		TestTrue(TEXT("Container has items"), ExpectedItems > 0);
		if (TestNotNull(TEXT("Container item grid widget"), Grid))
		{
			TestEqual(TEXT("Grid shows every container item"), Grid->GetNumDisplayedItems(), ExpectedItems);
		}
	});

	// 3a. Moving items between the container and the player inventory (server RPC path, standalone PIE).
	Step([this, State]()
	{
		ATypicalShooterPlayerController* PC = State->Controller.Get();
		UInventoryComponent* ContainerInventory = State->Container->InventoryComponent;
		UInventoryComponent* PlayerInventory = PC->GetPawn()->FindComponentByClass<UInventoryComponent>();
		const UInventoryGridWidget* PlayerGrid = FindPawnInventoryGrid(State->World.Get());
		if (!TestNotNull(TEXT("Player has an inventory"), PlayerInventory) || !TestNotNull(TEXT("HUD has a player inventory grid"), PlayerGrid))
		{
			return;
		}
		State->NumContainerItems = ContainerInventory->GetNumItems();
		TestEqual(TEXT("Player inventory starts empty"), PlayerInventory->GetNumItems(), 0);
		TestEqual(TEXT("Player grid starts empty"), PlayerGrid->GetNumDisplayedItems(), 0);

		UInventoryItem* First = ContainerInventory->GetPlacedItems()[0].Key;
		PC->RequestTransferItem(ContainerInventory, First, PlayerInventory, FHhAxialCoord(0, 0));
		TestEqual(TEXT("Transfer: item left the container"), ContainerInventory->GetNumItems(), State->NumContainerItems - 1);
		TestNotNull(TEXT("Transfer: item is in the player's centre cell"), PlayerInventory->GetItemAt(FHhAxialCoord(0, 0)));
		TestEqual(TEXT("Player grid shows the item"), PlayerGrid->GetNumDisplayedItems(), 1);
	});
	Step([this, State]()
	{
		ATypicalShooterPlayerController* PC = State->Controller.Get();
		UInventoryComponent* ContainerInventory = State->Container->InventoryComponent;
		UInventoryComponent* PlayerInventory = PC->GetPawn()->FindComponentByClass<UInventoryComponent>();
		UInventoryUISubsystem* UISubsystem = PC->GetLocalPlayer()->GetSubsystem<UInventoryUISubsystem>();
		if (ContainerInventory->GetNumItems() == 0 || !TestNotNull(TEXT("UI subsystem"), UISubsystem))
		{
			return;
		}

		UInventoryItem* Selected = ContainerInventory->GetPlacedItems()[0].Key;
		UISubsystem->SelectItem(Selected);
		TestTrue(TEXT("Selected item is displayed by the info panel"), UISubsystem->GetDisplayedItem() == Selected);
		PC->TakeSelectedItem();
		TestEqual(TEXT("Take: player got the selected item"), PlayerInventory->GetNumItems(), 2);
		TestNull(TEXT("Take: the taken item is no longer selected"), UISubsystem->GetSelectedItem());

		PC->TakeAllFromOpenedContainer();
		TestEqual(TEXT("Take All: container is empty"), ContainerInventory->GetNumItems(), 0);
		TestEqual(TEXT("Take All: player has every item"), PlayerInventory->GetNumItems(), State->NumContainerItems);
	});

	Tap(EKeys::E);
	Step([this, State]()
	{
		ATypicalShooterPlayerController* PC = State->Controller.Get();
		TestFalse(TEXT("Second E closes the inventory"), PC->IsInventoryOpened());
		TestNull(TEXT("Second E closes the container"), PC->GetOpenedItemsContainer());
		TestFalse(TEXT("Container grid hidden"), IsWidgetShown(State->GetHUD(), TEXT("BP_ContainerGrid")));
	});

	// 3b. The server refuses transfers into a container the pawn is not next to.
	Step([this, State]()
	{
		ATypicalShooterPlayerController* PC = State->Controller.Get();
		AItemsContainer* Container = State->Container.Get();
		UInventoryComponent* PlayerInventory = PC->GetPawn()->FindComponentByClass<UInventoryComponent>();
		if (PlayerInventory->GetNumItems() == 0)
		{
			return;
		}

		const FVector ContainerLocation = Container->GetActorLocation();
		Container->SetActorLocation(ContainerLocation + FVector(20000.f, 0.f, 0.f));
		TestFalse(TEXT("Far container is not accessible"), PC->CanAccessInventory(Container->InventoryComponent));
		TestTrue(TEXT("Own inventory is accessible"), PC->CanAccessInventory(PlayerInventory));

		const int32 Before = PlayerInventory->GetNumItems();
		PC->RequestTransferItem(PlayerInventory, PlayerInventory->GetPlacedItems()[0].Key, Container->InventoryComponent, FHhAxialCoord(0, 0));
		TestEqual(TEXT("Transfer into a far container is rejected"), PlayerInventory->GetNumItems(), Before);
		TestEqual(TEXT("Far container stays empty"), Container->InventoryComponent->GetNumItems(), 0);
		Container->SetActorLocation(ContainerLocation);
	});

	// 3c. Gamepad: X interacts, the D-pad moves the cursor, A picks up / puts down, RB switches grids, Y takes all,
	//     B closes; View toggles the inventory. The player now holds every item, the container is empty.
	Step([]() {}, 0.5f); // let the interaction overlap see the container again
	Tap(EKeys::Gamepad_FaceButton_Left);
	Step([this, State]()
	{
		ATypicalShooterPlayerController* PC = State->Controller.Get();
		UInventoryUISubsystem* UI = PC->GetLocalPlayer()->GetSubsystem<UInventoryUISubsystem>();
		TestTrue(TEXT("Gamepad X at a container opens the inventory"), PC->IsInventoryOpened());
		TestTrue(TEXT("Gamepad X at a container opens that container"), PC->GetOpenedItemsContainer() == State->Container.Get());
		TestTrue(TEXT("Cursor starts in the player's inventory"),
			UInventoryComponent::FindInventory(UI->GetCursorContainer()) == PC->GetPawn()->FindComponentByClass<UInventoryComponent>());
		TestTrue(TEXT("Cursor starts in the centre"), UI->GetCursorLocation() == FHhAxialCoord(0, 0));
	});
	Tap(EKeys::Gamepad_DPad_Right);
	Step([this, State]()
	{
		UInventoryUISubsystem* UI = State->Controller->GetLocalPlayer()->GetSubsystem<UInventoryUISubsystem>();
		TestTrue(TEXT("D-pad right moves the cursor to (1,0)"), UI->GetCursorLocation() == FHhAxialCoord(1, 0));
		TestTrue(TEXT("D-pad activates the cursor"), UI->IsCursorActive());
		const UInventoryGridWidget* PlayerGrid = FindPawnInventoryGrid(State->World.Get());
		if (TestNotNull(TEXT("Player grid"), PlayerGrid))
		{
			TestEqual(TEXT("Player grid draws the cursor"), PlayerGrid->GetSlateGrid()->GetCellHighlight(FHhAxialCoord(1, 0)), EInventoryCellHighlight::Cursor);
		}
	});
	Tap(EKeys::Gamepad_DPad_Left);
	Tap(EKeys::Gamepad_FaceButton_Bottom);
	Step([this, State]()
	{
		ATypicalShooterPlayerController* PC = State->Controller.Get();
		UInventoryUISubsystem* UI = PC->GetLocalPlayer()->GetSubsystem<UInventoryUISubsystem>();
		UInventoryComponent* PlayerInventory = PC->GetPawn()->FindComponentByClass<UInventoryComponent>();
		TestTrue(TEXT("Gamepad A selects the item under the cursor"),
			UI->GetSelectedItem() && UI->GetSelectedItem() == PlayerInventory->GetItemAt(FHhAxialCoord(0, 0)));
	});
	Tap(EKeys::Gamepad_RightShoulder);
	Tap(EKeys::Gamepad_FaceButton_Bottom);
	Step([this, State]()
	{
		ATypicalShooterPlayerController* PC = State->Controller.Get();
		UInventoryUISubsystem* UI = PC->GetLocalPlayer()->GetSubsystem<UInventoryUISubsystem>();
		UInventoryComponent* PlayerInventory = PC->GetPawn()->FindComponentByClass<UInventoryComponent>();
		TestTrue(TEXT("RB moves the cursor to the container"), UInventoryComponent::FindInventory(UI->GetCursorContainer()) == State->Container->InventoryComponent);
		TestEqual(TEXT("Gamepad A puts the selected item into the container"), State->Container->InventoryComponent->GetNumItems(), 1);
		TestNotNull(TEXT("...at the cursor cell"), State->Container->InventoryComponent->GetItemAt(FHhAxialCoord(0, 0)));
		TestEqual(TEXT("Player gave one item away"), PlayerInventory->GetNumItems(), State->NumContainerItems - 1);
		TestNull(TEXT("Nothing selected after the move"), UI->GetSelectedItem());
		TestFalse(TEXT("Gamepad A did not jump (inventory input has priority)"), PC->GetPawn()->GetMovementComponent()->IsFalling());
	});
	Tap(EKeys::Gamepad_FaceButton_Top);
	Step([this, State]()
	{
		ATypicalShooterPlayerController* PC = State->Controller.Get();
		TestEqual(TEXT("Gamepad Y takes all"), State->Container->InventoryComponent->GetNumItems(), 0);
		TestEqual(TEXT("Player has every item again"), PC->GetPawn()->FindComponentByClass<UInventoryComponent>()->GetNumItems(), State->NumContainerItems);
	});
	Tap(EKeys::Gamepad_FaceButton_Right);
	Step([this, State]()
	{
		ATypicalShooterPlayerController* PC = State->Controller.Get();
		TestFalse(TEXT("Gamepad B with nothing selected closes the inventory"), PC->IsInventoryOpened());
		TestNull(TEXT("...and the container"), PC->GetOpenedItemsContainer());
		TestNull(TEXT("Closed inventory: no cursor"), PC->GetLocalPlayer()->GetSubsystem<UInventoryUISubsystem>()->GetCursorContainer());
	});
	Tap(EKeys::Gamepad_Special_Left);
	Step([this, State]() { TestTrue(TEXT("Gamepad View opens the inventory"), State->Controller->IsInventoryOpened()); });
	Tap(EKeys::Gamepad_Special_Left);
	Step([this, State]() { TestFalse(TEXT("Gamepad View closes the inventory"), State->Controller->IsInventoryOpened()); });

	// 4. Movement and jump work with the inventory closed.
	Step([State]() { State->SavedLocation = State->Controller->GetPawn()->GetActorLocation(); }, 0.0f);
	Step([State]() { State->SendKey(EKeys::W, IE_Pressed); }, 0.0f);
	Step([State]() { State->SendKey(EKeys::W, IE_Released); }, 0.5f);
	Step([this, State]()
	{
		const FVector Now = State->Controller->GetPawn()->GetActorLocation();
		TestTrue(FString::Printf(TEXT("W moves the character (moved %.1f)"), FVector::Dist2D(Now, State->SavedLocation)),
			FVector::Dist2D(Now, State->SavedLocation) > 20.f);
		State->SavedLocation = Now;
	}, 0.1f);
	Step([State]() { State->SendKey(EKeys::SpaceBar, IE_Pressed); }, 0.0f);
	Step([this, State]()
	{
		State->SendKey(EKeys::SpaceBar, IE_Released);
		const float Rise = State->Controller->GetPawn()->GetActorLocation().Z - State->SavedLocation.Z;
		TestTrue(FString::Printf(TEXT("SpaceBar jumps (rose %.1f)"), Rise), Rise > 5.f);
	}, 0.2f);

	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
