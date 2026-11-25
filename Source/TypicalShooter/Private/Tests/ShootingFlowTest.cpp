// End-to-end shooting check in Play In Editor: the player fires primary / alternate attacks at a target dummy
// with real key events (UGameViewportClient::InputKey), reloads, and can't fire with the inventory open.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "Shooting/AttackAttributeSet.h"
#include "Shooting/TargetDummy.h"
#include "Tests/AutomationEditorCommon.h"
#include "TypicalShooterPlayerController.h"

namespace ShootingFlowTest
{
	struct FState
	{
		bool bReady = false;
		double WaitStarted = 0.0;
		TWeakObjectPtr<UWorld> World;
		TWeakObjectPtr<ATypicalShooterPlayerController> Controller;
		TWeakObjectPtr<UAbilitySystemComponent> Asc;
		TWeakObjectPtr<ATargetDummy> Dummy;
		float SavedDamage = 0.f;
		float SavedAmmo = 0.f;

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
			APawn* Pawn = Controller.IsValid() ? Controller->GetPawn() : nullptr;
			Asc = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Pawn);
			return Asc.IsValid() && Asc->GetSet<UAttackAttributeSet>() && Asc->GetActivatableAbilities().Num() > 0;
		}

		/** Target dummy 400 cm straight ahead of the player's view. */
		void SpawnDummyInFront()
		{
			FVector ViewLocation;
			FRotator ViewRotation;
			Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);
			ViewRotation.Pitch = 0.f;
			Controller->SetControlRotation(ViewRotation);
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			Dummy = World->SpawnActor<ATargetDummy>(ViewLocation + ViewRotation.Vector() * 400.f, ViewRotation, Params);
		}

		float Ammo(const EAttackSlot Slot) const
		{
			return Asc->GetNumericAttribute(UAttackAttributeSet::GetAttribute(Slot, EAttackAttribute::Ammo));
		}

		float Damage() const { return Dummy.IsValid() ? Dummy->GetTotalDamageTaken() : 0.f; }

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FShootingFlowTest, "TypicalShooter.Shooting.Flow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FShootingFlowTest::RunTest(const FString& Parameters)
{
	using namespace ShootingFlowTest;
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
	auto Tap = [&Step, State](const FKey Key)
	{
		Step([State, Key]() { State->SendKey(Key, IE_Pressed); }, 0.0f);
		Step([State, Key]() { State->SendKey(Key, IE_Released); }, 0.05f);
	};
	auto Save = [&Step, State]()
	{
		Step([State]() { State->SavedDamage = State->Damage(); State->SavedAmmo = State->Ammo(EAttackSlot::Primary); }, 0.0f);
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
			AddError(TEXT("PIE did not provide a player pawn with attack attributes and abilities within 15 s."));
			return true;
		}
		return false;
	}));

	// 1. Default attack stats are applied, a dummy stands in front of the player.
	Step([this, State]()
	{
		TestEqual(TEXT("Primary clip loaded from the defaults effect"), State->Ammo(EAttackSlot::Primary), 30.f);
		TestEqual(TEXT("Alternate clip loaded from the defaults effect"), State->Ammo(EAttackSlot::Alternate), 6.f);
		State->SpawnDummyInFront();
	}, 0.5f);

	// 2. LMB fires the primary attack at the dummy.
	Tap(EKeys::LeftMouseButton);
	Step([this, State]()
	{
		TestEqual(TEXT("LMB spends a primary round"), State->Ammo(EAttackSlot::Primary), 29.f);
		TestTrue(FString::Printf(TEXT("Primary projectile hits the dummy (damage %.1f)"), State->Damage()), State->Damage() > 0.f);
		State->SavedDamage = State->Damage();
	}, 0.4f);

	// 3. RMB fires the alternate attack (spread of projectiles).
	Tap(EKeys::RightMouseButton);
	Step([this, State]()
	{
		TestEqual(TEXT("RMB spends an alternate round"), State->Ammo(EAttackSlot::Alternate), 5.f);
		TestTrue(TEXT("Alternate projectiles hit the dummy"), State->Damage() > State->SavedDamage);
	}, 0.4f);

	// 4. R reloads both attacks.
	Tap(EKeys::R);
	Step([this, State]()
	{
		TestEqual(TEXT("R reloads the primary clip"), State->Ammo(EAttackSlot::Primary), 30.f);
		TestEqual(TEXT("R reloads the alternate clip"), State->Ammo(EAttackSlot::Alternate), 6.f);
	}, 2.5f);

	// 5. No firing with the inventory open.
	Tap(EKeys::Tab);
	Save();
	Tap(EKeys::LeftMouseButton);
	Step([this, State]()
	{
		TestTrue(TEXT("Inventory is open"), State->Controller->IsInventoryOpened());
		TestEqual(TEXT("LMB does not fire with the inventory open"), State->Ammo(EAttackSlot::Primary), State->SavedAmmo);
	}, 0.4f);
	Tap(EKeys::Tab);

	// 6. Gamepad right trigger fires the primary attack.
	Save();
	Tap(EKeys::Gamepad_RightTrigger);
	Step([this, State]()
	{
		TestEqual(TEXT("RT spends a primary round"), State->Ammo(EAttackSlot::Primary), State->SavedAmmo - 1.f);
	}, 0.4f);

	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
