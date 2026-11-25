// End-to-end power grid check in Play In Editor: the starting set fills all three receiver slots, Space jumps and
// Space in the air fires the air receiver straight down (its recoil is a double jump), and the player's inventory
// grid draws the power grid with slot marks; a level container shows its power grid items.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "EngineUtils.h"
#include "Engine/LocalPlayer.h"
#include "InventoryItem.h"
#include "ItemsContainer.h"
#include "UnrealClient.h"
#include "UI/InventoryUISubsystem.h"
#include "PowerGrid/DefaultLoadout.h"
#include "PowerGrid/PowerGridComponent.h"
#include "Tests/AutomationEditorCommon.h"
#include "TypicalShooterPlayerController.h"
#include "UI/Slate/InventoryGrid.h"
#include "UI/Widget/InventoryGridWidget.h"
#include "UObject/UObjectIterator.h"

namespace PowerGridFlowTest
{
	struct FState
	{
		bool bReady = false;
		double WaitStarted = 0.0;
		TWeakObjectPtr<UWorld> World;
		TWeakObjectPtr<ATypicalShooterPlayerController> Controller;
		TWeakObjectPtr<ACharacter> Character;
		TWeakObjectPtr<AItemsContainer> Container;
		float SavedVelocityZ = 0.f;

		bool TryResolve()
		{
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				if (Context.WorldType == EWorldType::PIE && Context.World())
				{
					World = Context.World();
				}
			}
			Controller = World.IsValid() ? Cast<ATypicalShooterPlayerController>(World->GetFirstPlayerController()) : nullptr;
			Character = Controller.IsValid() ? Cast<ACharacter>(Controller->GetPawn()) : nullptr;
			const UPowerGridComponent* Grid = Character.IsValid() ? Character->FindComponentByClass<UPowerGridComponent>() : nullptr;
			return Grid && Grid->GetNodeItems().Num() > 0 && Character->GetCharacterMovement()->IsMovingOnGround();
		}

		const UPowerGridComponent* Grid() const { return Character->FindComponentByClass<UPowerGridComponent>(); }
		float VelocityZ() const { return Character->GetCharacterMovement()->Velocity.Z; }

		const UInventoryGridWidget* FindPawnGrid() const
		{
			for (TObjectIterator<UInventoryGridWidget> It; It; ++It)
			{
				if (It->bShowOwningPawnInventory && It->GetWorld() == World.Get() && It->GetSlateGrid().IsValid())
				{
					return *It;
				}
			}
			return nullptr;
		}

		void SendKey(const FKey& Key, const EInputEvent Event) const
		{
			UGameViewportClient* ViewportClient = World.IsValid() ? World->GetGameViewport() : nullptr;
			if (ViewportClient)
			{
				const FInputDeviceId Device = IPlatformInputDeviceMapper::Get().GetDefaultInputDevice();
				ViewportClient->InputKey(FInputKeyEventArgs(ViewportClient->Viewport, Device, Key, Event, FPlatformTime::Cycles64()));
			}
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerGridFlowTest, "TypicalShooter.PowerGrid.Flow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPowerGridFlowTest::RunTest(const FString& Parameters)
{
	using namespace PowerGridFlowTest;
	const TSharedRef<FState> State = MakeShared<FState>();

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

	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, State]()
	{
		State->WaitStarted = State->WaitStarted == 0.0 ? FPlatformTime::Seconds() : State->WaitStarted;
		if (State->TryResolve())
		{
			State->bReady = true;
			return true;
		}
		if (FPlatformTime::Seconds() - State->WaitStarted > 15.0)
		{
			AddError(TEXT("PIE did not provide a grounded player character with a power grid within 15 s."));
			return true;
		}
		return false;
	}));

	// 1. The starting set: every receiver slot has a working receiver.
	Step([this, State]()
	{
		const UPowerGridComponent* Grid = State->Grid();
		TestEqual(TEXT("Starting set: core + three receivers"), Grid->GetNodeItems().Num(), DefaultLoadout::MakeStartingItems().Num());
		TestNotNull(TEXT("Primary slot works"), Grid->GetActiveReceiver(EReceiverSlot::Primary));
		TestNotNull(TEXT("Alternate slot works"), Grid->GetActiveReceiver(EReceiverSlot::Alternate));
		TestNotNull(TEXT("Air slot works (double jump receiver)"), Grid->GetActiveReceiver(EReceiverSlot::Air));
	}, 0.5f);

	// 2. Space jumps; Space in the air fires down and the recoil pushes the character up.
	Step([State]() { State->SendKey(EKeys::SpaceBar, IE_Pressed); }, 0.0f);
	Step([State]() { State->SendKey(EKeys::SpaceBar, IE_Released); }, 0.1f);
	Step([this, State]()
	{
		TestTrue(TEXT("Space jumped"), State->Character->GetCharacterMovement()->IsFalling());
		State->SavedVelocityZ = State->VelocityZ();
		State->SendKey(EKeys::SpaceBar, IE_Pressed);
	}, 0.25f);
	Step([this, State]()
	{
		State->SendKey(EKeys::SpaceBar, IE_Released);
		TestTrue(FString::Printf(TEXT("Space in the air: the air shot's recoil is a double jump (vz %.0f -> %.0f)"), State->SavedVelocityZ, State->VelocityZ()),
			State->VelocityZ() > State->SavedVelocityZ + 400.f);
	}, 0.05f);

	// 3. The inventory grid draws the player's power grid with the receiver slot marks.
	Step([this, State]()
	{
		const UInventoryGridWidget* Widget = State->FindPawnGrid();
		if (TestNotNull(TEXT("HUD has the player's inventory grid"), Widget))
		{
			const FPowerGridDisplay& Display = Widget->GetSlateGrid()->GetPowerGrid();
			TestEqual(TEXT("Grid draws every power item"), Display.Cells.Num(), DefaultLoadout::MakeStartingItems().Num());
			TestTrue(TEXT("Grid marks the receiver slots"), Display.bShowSlots);
			TestEqual(TEXT("Grid draws the core's links to the three receivers"), Display.Links.Num(), 3);
		}
	}, 1.f);

	// 4. The level container with power grid test items: walk up, E opens it, its grid draws its own power grid; the
	//    damage amplifier is shown in the info panel. A screenshot is saved for a visual check (rendering editors only).
	Step([this, State]()
	{
		for (TActorIterator<AItemsContainer> It(State->World.Get()); It; ++It)
		{
			const UPowerGridComponent* Grid = It->FindComponentByClass<UPowerGridComponent>();
			if (Grid && Grid->GetNodeItems().Num() > 0)
			{
				State->Container = *It;
				break;
			}
		}
		if (!TestTrue(TEXT("A level container holds power grid items"), State->Container.IsValid()))
		{
			return;
		}
		const FVector Target = State->Container->GetActorLocation() + FVector(-60.f, 0.f, 50.f);
		State->Character->TeleportTo(Target, State->Character->GetActorRotation());
	}, 1.f);
	Step([State]() { State->SendKey(EKeys::E, IE_Pressed); }, 0.5f);
	Step([State]() { State->SendKey(EKeys::E, IE_Released); }, 0.1f);
	Step([this, State]()
	{
		if (!State->Container.IsValid())
		{
			return;
		}
		TestTrue(TEXT("E opens the power grid container"), State->Controller->GetOpenedItemsContainer() == State->Container.Get());
		const UPowerGridComponent* Grid = State->Container->FindComponentByClass<UPowerGridComponent>();
		UInventoryItem* Amplifier = nullptr;
		for (UInventoryItem* Item : Grid->GetNodeItems())
		{
			Amplifier = Item && Item->GetItemName().ToString().Contains(TEXT("Amplifier")) ? Item : Amplifier;
		}
		TestNotNull(TEXT("The container holds the damage amplifier"), Amplifier);
		int32 DrawnContainerCells = 0;
		for (TObjectIterator<UInventoryGridWidget> It; It; ++It)
		{
			if (!It->bShowOwningPawnInventory && It->GetWorld() == State->World.Get() && It->GetSlateGrid().IsValid())
			{
				DrawnContainerCells = FMath::Max(DrawnContainerCells, It->GetSlateGrid()->GetPowerGrid().Cells.Num());
			}
		}
		TestEqual(TEXT("The container grid draws its power items"), DrawnContainerCells, Grid->GetNodeItems().Num());		if (UInventoryUISubsystem* UI = State->Controller->GetLocalPlayer()->GetSubsystem<UInventoryUISubsystem>())
		{
			UI->SelectItem(Amplifier);
		}
		FScreenshotRequest::RequestScreenshot(TEXT("PowerGridFlow"), true, false);
	}, 0.3f);
	Step([]() {}, 1.f); // let the screenshot be taken

	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

#endif
