// End-to-end shooting check in Play In Editor: the player fires primary / alternate attacks at a target dummy
// with real key events (UGameViewportClient::InputKey) and can't fire with the inventory open. Attacks have no clips.

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
#include "Shooting/AmmoAttributeSet.h"
#include "Shooting/AttackAttributeSet.h"
#include "Shooting/ShootingTags.h"
#include "Shooting/ShotProjectile.h"
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
		/** Projectiles spawned in the PIE world since the pawn was resolved. */
		int32 Projectiles = 0;
		int32 SavedProjectiles = 0;
		float SavedBulletReserve = 0.f;

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

		float Attribute(const EAttackSlot Slot, const EAttackAttribute Attribute) const
		{
			return Asc->GetNumericAttribute(UAttackAttributeSet::GetAttribute(Slot, Attribute));
		}

		float BulletReserve() const
		{
			return Asc->GetNumericAttribute(UAmmoAttributeSet::GetReserveAttribute(ShootingTags::Ammo_Bullet));
		}

		static void CountProjectiles(const TSharedRef<FState>& State)
		{
			State->World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateLambda([State](AActor* Spawned)
			{
				State->Projectiles += Spawned && Spawned->IsA<AShotProjectile>() ? 1 : 0;
			}));
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
		Step([State]() { State->SavedDamage = State->Damage(); State->SavedProjectiles = State->Projectiles; }, 0.0f);
	};

	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, State]()
	{
		State->WaitStarted = State->WaitStarted == 0.0 ? FPlatformTime::Seconds() : State->WaitStarted;
		if (State->TryResolve())
		{
			State->bReady = true;
			FState::CountProjectiles(State);
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
		TestEqual(TEXT("Primary damage from the defaults effect"), State->Attribute(EAttackSlot::Primary, EAttackAttribute::DamagePhysical), 10.f);
		TestEqual(TEXT("Alternate damage from the defaults effect"), State->Attribute(EAttackSlot::Alternate, EAttackAttribute::DamagePhysical), 6.f);
		State->SpawnDummyInFront();
	}, 0.5f);

	// 2. LMB fires the primary attack at the dummy.
	Save();
	Tap(EKeys::LeftMouseButton);
	Step([this, State]()
	{
		TestEqual(TEXT("LMB fires one primary projectile"), State->Projectiles - State->SavedProjectiles, 1);
		TestTrue(FString::Printf(TEXT("Primary projectile hits the dummy (damage %.1f)"), State->Damage()), State->Damage() > 0.f);
	}, 0.4f);

	// 3. RMB fires the alternate attack (spread of projectiles).
	Save();
	Tap(EKeys::RightMouseButton);
	Step([this, State]()
	{
		TestEqual(TEXT("RMB fires the seven-pellet alternate attack"), State->Projectiles - State->SavedProjectiles, 7);
		TestTrue(TEXT("Alternate projectiles hit the dummy"), State->Damage() > State->SavedDamage);
		State->SavedBulletReserve = State->BulletReserve();
	}, 0.4f);

	// 4. R no longer reloads anything (clips and reloads were removed, power grid R9a).
	Save();
	Tap(EKeys::R);
	Step([this, State]()
	{
		TestEqual(TEXT("R fires nothing"), State->Projectiles, State->SavedProjectiles);
		TestEqual(TEXT("R takes nothing from the reserve"), State->BulletReserve(), State->SavedBulletReserve);
	}, 0.4f);

	// 5. No firing with the inventory open.
	Tap(EKeys::Tab);
	Save();
	Tap(EKeys::LeftMouseButton);
	Step([this, State]()
	{
		TestTrue(TEXT("Inventory is open"), State->Controller->IsInventoryOpened());
		TestEqual(TEXT("LMB does not fire with the inventory open"), State->Projectiles, State->SavedProjectiles);
	}, 0.4f);
	Tap(EKeys::Tab);

	// 6. Gamepad right trigger fires the primary attack.
	Save();
	Tap(EKeys::Gamepad_RightTrigger);
	Step([this, State]()
	{
		TestEqual(TEXT("RT fires one primary projectile"), State->Projectiles - State->SavedProjectiles, 1);
	}, 0.4f);

	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
