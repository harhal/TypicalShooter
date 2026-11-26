// Recording scenario (not part of the TypicalShooter. suites): in PIE on Lvl_FirstPerson the player turns to the nearest
// enemy of the first wave and shoots it dead with the rifle (LMB through UGameViewportClient::InputKey). Runs with a fixed
// 1/30 s time step and saves every frame to Saved/Recordings/EnemyKill/frame_NNNN.png (rendering editor only):
//   UnrealEditor.exe <uproject> -ExecCmds="Automation RunTests TypicalShooterRecordings.EnemyKill;Quit" -unattended -nopause
//   ffmpeg -framerate 30 -i Saved/Recordings/EnemyKill/frame_%04d.png -c:v libx264 -pix_fmt yuv420p EnemyKill.mp4

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Enemies/EnemyCharacter.h"
#include "Enemies/EnemyDirectorSubsystem.h"
#include "Enemies/EnemyWaveSpawner.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "HAL/FileManager.h"
#include "InputKeyEventArgs.h"
#include "Misc/App.h"
#include "Misc/Paths.h"
#include "Tests/AutomationEditorCommon.h"
#include "UnrealClient.h"

namespace RecordingEnemyKill
{
	constexpr double FrameTime = 1.0 / 30.0;
	constexpr float TurnSpeed = 4.f;      // RInterpTo speed while turning to the enemy
	constexpr float FireAngle = 2.f;      // start firing when the aim is this close (degrees)
	constexpr int32 FramesAfterKill = 45; // 1.5 s epilogue
	constexpr int32 MaxFrames = 900;      // 30 s safety limit

	enum class EPhase : uint8 { WaitPIE, WaitEnemy, Hunt, Epilogue, Done };

	struct FState
	{
		EPhase Phase = EPhase::WaitPIE;
		TWeakObjectPtr<UWorld> World;
		TWeakObjectPtr<APlayerController> Controller;
		TWeakObjectPtr<AEnemyCharacter> Target;
		bool bFiring = false;
		bool bKilled = false;
		int32 Frame = 0;
		int32 EpilogueFrames = 0;
		int32 Steps = 0;
		FString Folder;

		bool TryResolve()
		{
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				if (Context.WorldType == EWorldType::PIE && Context.World())
				{
					World = Context.World();
				}
			}
			Controller = World.IsValid() ? World->GetFirstPlayerController() : nullptr;
			return Controller.IsValid() && Controller->GetPawn();
		}

		AEnemyCharacter* NearestEnemy() const
		{
			const UEnemyDirectorSubsystem* Director = World->GetSubsystem<UEnemyDirectorSubsystem>();
			AEnemyCharacter* Nearest = nullptr;
			for (AEnemyCharacter* Enemy : Director ? Director->GetEnemies() : TArray<AEnemyCharacter*>())
			{
				if (!Nearest || FVector::Dist(Enemy->GetActorLocation(), Controller->GetPawn()->GetActorLocation())
					< FVector::Dist(Nearest->GetActorLocation(), Controller->GetPawn()->GetActorLocation()))
				{
					Nearest = Enemy;
				}
			}
			return Nearest;
		}

		void SendKey(const FKey& Key, const EInputEvent Event) const
		{
			if (UGameViewportClient* ViewportClient = World.IsValid() ? World->GetGameViewport() : nullptr)
			{
				const FInputDeviceId Device = IPlatformInputDeviceMapper::Get().GetDefaultInputDevice();
				ViewportClient->InputKey(FInputKeyEventArgs(ViewportClient->Viewport, Device, Key, Event, FPlatformTime::Cycles64()));
			}
		}

		void SetFiring(const bool bFire)
		{
			if (bFire != bFiring)
			{
				bFiring = bFire;
				SendKey(EKeys::LeftMouseButton, bFire ? IE_Pressed : IE_Released);
			}
		}

		/** Turns the view towards the target; returns the remaining angle in degrees. */
		float AimAtTarget() const
		{
			FVector ViewLocation;
			FRotator ViewRotation;
			Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);
			const FRotator Desired = (Target->GetActorLocation() - ViewLocation).Rotation();
			Controller->SetControlRotation(FMath::RInterpTo(Controller->GetControlRotation(), Desired, FrameTime, TurnSpeed));
			return FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(ViewRotation.Vector() | Desired.Vector(), -1.f, 1.f)));
		}

		void CaptureFrame()
		{
			FScreenshotRequest::RequestScreenshot(FPaths::Combine(Folder, FString::Printf(TEXT("frame_%04d.png"), Frame++)), true, false);
		}

		/** One frame of the scenario; true when done. */
		bool Step()
		{
			if (++Steps > 3000)
			{
				return true; // PIE or the wave never came
			}
			switch (Phase)
			{
			case EPhase::WaitPIE:
				Phase = TryResolve() ? EPhase::WaitEnemy : EPhase::WaitPIE;
				return false;
			case EPhase::WaitEnemy:
				Target = NearestEnemy();
				Phase = Target.IsValid() ? EPhase::Hunt : EPhase::WaitEnemy;
				CaptureFrame();
				return false;
			case EPhase::Hunt:
				if (!Target.IsValid() || Target->IsDead())
				{
					SetFiring(false);
					bKilled = true;
					Phase = EPhase::Epilogue;
				}
				else
				{
					SetFiring(AimAtTarget() <= FireAngle);
				}
				CaptureFrame();
				return Frame > MaxFrames;
			case EPhase::Epilogue:
				CaptureFrame();
				Phase = ++EpilogueFrames >= FramesAfterKill ? EPhase::Done : EPhase::Epilogue;
				return false;
			default:
				return true;
			}
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRecordingEnemyKillTest, "TypicalShooterRecordings.EnemyKill",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FRecordingEnemyKillTest::RunTest(const FString& Parameters)
{
	using namespace RecordingEnemyKill;
	const TSharedRef<FState> State = MakeShared<FState>();
	State->Folder = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("Recordings/EnemyKill"));
	IFileManager::Get().DeleteDirectory(*State->Folder, false, true);
	IFileManager::Get().MakeDirectory(*State->Folder, true);

	AEnemyWaveSpawner::SetWavesEnabled(true);
	FApp::SetUseFixedTimeStep(true);
	FApp::SetFixedDeltaTime(FrameTime);
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, State]()
	{
		const bool bDone = State->Step();
		if (bDone || State->Phase == EPhase::Done)
		{
			TestTrue(TEXT("The player killed the enemy"), State->bKilled);
			AddInfo(FString::Printf(TEXT("Recorded %d frames to %s"), State->Frame, *State->Folder));
			return true;
		}
		return false;
	}));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([]()
	{
		FApp::SetUseFixedTimeStep(false);
		return true;
	}));
	return true;
}

#endif
