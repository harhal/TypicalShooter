// End-to-end coordination check in Play In Editor on Lvl_FirstPerson: the wave of four (third wave) shares the attack
// tokens, takes roles by the weights, spreads around the player with the flanker outside his view, and is recomputed at
// once when one of them dies.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Algo/Count.h"
#include "Enemies/Coordination/EnemyCoordinationSettings.h"
#include "Enemies/Coordination/EnemySectors.h"
#include "Enemies/EnemyAIController.h"
#include "Enemies/EnemyCharacter.h"
#include "Enemies/EnemyDirectorSubsystem.h"
#include "Enemies/EnemyWaveSpawner.h"
#include "Engine/Engine.h"
#include "Engine/HitResult.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationData.h"
#include "GameFramework/PlayerController.h"
#include "Shooting/ShootingTags.h"
#include "Shooting/ShotDamage.h"
#include "Shooting/ShotDamageReceiver.h"
#include "Tests/AutomationEditorCommon.h"

namespace EnemiesCoordinationFlowTest
{
	struct FState
	{
		TWeakObjectPtr<UWorld> World;
		TWeakObjectPtr<APawn> Player;
		TWeakObjectPtr<AEnemyWaveSpawner> Spawner;
		double StepStarted = 0.0;
		bool bFailed = false;
		int32 MaxFiring = 0;

		bool TryResolve()
		{
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				World = Context.WorldType == EWorldType::PIE && Context.World() ? Context.World() : World.Get();
			}
			if (!World.IsValid())
			{
				return false;
			}
			const APlayerController* Controller = World->GetFirstPlayerController();
			Player = Controller ? Controller->GetPawn() : nullptr;
			for (TActorIterator<AEnemyWaveSpawner> It(World.Get()); It; ++It)
			{
				Spawner = *It;
			}
			return Player.IsValid() && Spawner.IsValid();
		}

		const UEnemyDirectorSubsystem* Director() const
		{
			return World.IsValid() ? World->GetSubsystem<UEnemyDirectorSubsystem>() : nullptr;
		}

		TArray<AEnemyCharacter*> Enemies() const { return Director() ? Director()->GetEnemies() : TArray<AEnemyCharacter*>(); }

		int32 CountRole(const EEnemyRole Role) const
		{
			return Algo::CountIf(Enemies(), [this, Role](const AEnemyCharacter* E)
			{
				const FEnemyAssignment* A = Director()->GetAssignment(E);
				return A && A->Role == Role;
			});
		}

		int32 CountTokens() const
		{
			return Algo::CountIf(Enemies(), [this](const AEnemyCharacter* E) { return Director()->HasAttackToken(E); });
		}

		float YawOf(const AEnemyCharacter* Enemy) const { return EnemySectors::YawAround(Player->GetActorLocation(), Enemy->GetActorLocation()); }

		float MinAngleBetweenEnemies() const
		{
			const TArray<AEnemyCharacter*> Live = Enemies();
			float Min = 360.f;
			for (int32 I = 0; I < Live.Num(); ++I)
			{
				for (int32 J = I + 1; J < Live.Num(); ++J)
				{
					Min = FMath::Min(Min, FMath::Abs(FMath::FindDeltaAngleDegrees(YawOf(Live[I]), YawOf(Live[J]))));
				}
			}
			return Min;
		}

		bool FlankerOutOfView(const float Tolerance) const
		{
			const float Half = GetDefault<UEnemyCoordinationSettings>()->Sectors.FlankViewHalfAngle;
			for (const AEnemyCharacter* Enemy : Enemies())
			{
				const FEnemyAssignment* A = Director()->GetAssignment(Enemy);
				if (A && A->Role == EEnemyRole::Flank)
				{
					return FMath::Abs(FMath::FindDeltaAngleDegrees(YawOf(Enemy), Player->GetViewRotation().Yaw)) > Half - Tolerance;
				}
			}
			return false;
		}

		/** One line per enemy: role, token, actual / sector yaw, distance, stand point distance, move status. */
		FString Describe() const
		{
			FString Text;
			for (const AEnemyCharacter* Enemy : Enemies())
			{
				const FEnemyAssignment* A = Director()->GetAssignment(Enemy);
				const AEnemyAIController* Controller = Cast<AEnemyAIController>(Enemy->GetController());
				const UPathFollowingComponent* Path = Controller ? Controller->GetPathFollowingComponent() : nullptr;
				const FNavPathSharedPtr NavPath = Path ? Path->GetPath() : nullptr;
				// What a 60 cm step towards the stand point would hit (stuck enemies).
				FString Blocker = TEXT("-");
				if (Controller)
				{
					const FVector From = Enemy->GetActorLocation();
					const FVector To = From + (Controller->GetStandPoint() - From).GetSafeNormal2D() * 60.f;
					FHitResult Hit;
					FCollisionQueryParams Params(SCENE_QUERY_STAT(CoordinationFlowProbe), false, Enemy);
					if (World->SweepSingleByChannel(Hit, From, To, FQuat::Identity, ECC_Pawn,
						Enemy->GetCapsuleComponent()->GetCollisionShape(), Params))
					{
						Blocker = FString::Printf(TEXT("%s/%s start-pen %d"), *GetNameSafe(Hit.GetActor()), *GetNameSafe(Hit.GetComponent()), Hit.bStartPenetrating);
					}
				}
				const FString States = Controller
					? FString::JoinBy(Controller->GetActiveStateNames(), TEXT("/"), [](const FName Name) { return Name.ToString(); }) : FString();
				Text += FString::Printf(TEXT("\n  %s%s [%s] yaw %.0f sector %.0f dist %.0f stand %.0f move %d speed %.0f path %d pts at %s blocked by %s mode %d"),
					A ? *UEnum::GetDisplayValueAsText(A->Role).ToString() : TEXT("-"), Director()->HasAttackToken(Enemy) ? TEXT("*") : TEXT(""),
					*States, YawOf(Enemy), A ? A->SectorYaw : 0.f, FVector::Dist2D(Enemy->GetActorLocation(), Player->GetActorLocation()),
					Controller ? FVector::Dist2D(Controller->GetStandPoint(), Player->GetActorLocation()) : 0.0,
					Controller ? int32(Controller->GetMoveStatus()) : -1, Enemy->GetVelocity().Size2D(),
					NavPath.IsValid() ? NavPath->GetPathPoints().Num() : 0, *Enemy->GetActorLocation().ToCompactString(), *Blocker,
					int32(Enemy->GetCharacterMovement()->MovementMode));
			}
			return Text;
		}

		void Kill(AEnemyCharacter* Enemy) const
		{
			FShotDamage Damage;
			Damage.Amounts.Add(ShootingTags::Damage_Type_Physical, 100000.f);
			IShotDamageReceiver::FindOnActor(Enemy)->ReceiveShotDamage(Damage, FHitResult());
		}

		void KillAll() const
		{
			for (AEnemyCharacter* Enemy : Enemies())
			{
				Kill(Enemy);
			}
		}
	};

	/** Polls Condition every frame until it holds; error after Timeout seconds (later steps are skipped). */
	void WaitUntil(FAutomationTestBase* Test, const TSharedRef<FState>& State, const FString& What, const double Timeout,
		TFunction<bool()> Condition)
	{
		ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([Test, State, What, Timeout, Condition = MoveTemp(Condition)]()
		{
			if (State->bFailed)
			{
				return true;
			}
			const double Now = FPlatformTime::Seconds();
			State->StepStarted = State->StepStarted == 0.0 ? Now : State->StepStarted;
			if (Condition())
			{
				Test->AddInfo(FString::Printf(TEXT("%s (%.1f s)"), *What, Now - State->StepStarted));
				State->StepStarted = 0.0;
				return true;
			}
			if (Now - State->StepStarted > Timeout)
			{
				Test->AddError(FString::Printf(TEXT("Not within %.0f s: %s"), Timeout, *What));
				State->bFailed = true;
				return true;
			}
			return false;
		}));
	}

	/** Runs Sample every frame for Seconds. */
	void Observe(const TSharedRef<FState>& State, const double Seconds, TFunction<void()> Sample)
	{
		ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([State, Seconds, Sample = MoveTemp(Sample)]()
		{
			if (State->bFailed)
			{
				return true;
			}
			const double Now = FPlatformTime::Seconds();
			State->StepStarted = State->StepStarted == 0.0 ? Now : State->StepStarted;
			Sample();
			const bool bDone = Now - State->StepStarted >= Seconds;
			State->StepStarted = bDone ? 0.0 : State->StepStarted;
			return bDone;
		}));
	}

	void KillWave(FAutomationTestBase* Test, const TSharedRef<FState>& State, const int32 Size)
	{
		WaitUntil(Test, State, FString::Printf(TEXT("A wave of %d is up"), Size), 20.0, [State, Size]() { return State->Enemies().Num() == Size; });
		WaitUntil(Test, State, FString::Printf(TEXT("The wave of %d is killed"), Size), 1.0, [State]()
		{
			State->KillAll();
			return State->Enemies().IsEmpty();
		});
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesCoordinationFlowTest, "TypicalShooter.Enemies.Coordination.Flow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FEnemiesCoordinationFlowTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesCoordinationFlowTest;
	const TSharedRef<FState> State = MakeShared<FState>();
	const UEnemyCoordinationSettings* Settings = GetDefault<UEnemyCoordinationSettings>();
	AEnemyWaveSpawner::SetWavesEnabled(true);
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));

	WaitUntil(this, State, TEXT("PIE has the player pawn and a wave spawner"), 15.0, [State]() { return State->TryResolve(); });
	KillWave(this, State, 2);
	KillWave(this, State, 3);
	WaitUntil(this, State, TEXT("The wave of four is up"), 20.0, [State]() { return State->Enemies().Num() == 4; });

	WaitUntil(this, State, TEXT("Roles by the weights: 2 pressure, 1 flank, 1 cover"), 2.0, [State]()
	{
		return State->CountRole(EEnemyRole::Pressure) == 2 && State->CountRole(EEnemyRole::Flank) == 1 && State->CountRole(EEnemyRole::Cover) == 1;
	});
	Observe(State, 8.0, [State]()
	{
		State->MaxFiring = FMath::Max(State->MaxFiring, int32(Algo::CountIf(State->Enemies(), [](const AEnemyCharacter* E) { return E->IsFiring(); })));
	});
	WaitUntil(this, State, TEXT("At most the group's tokens fired at once while watched (8 s)"), 0.5, [this, State, Settings]()
	{
		AddInfo(FString::Printf(TEXT("Most enemies firing at once: %d (tokens for 4: %d)"), State->MaxFiring, Settings->GetTokenCount(4)));
		return State->MaxFiring <= Settings->GetTokenCount(4) && State->MaxFiring > 0;
	});
	WaitUntil(this, State, TEXT("The four spread around the player (>= 40 degrees apart)"), 15.0, [this, State]()
	{
		static double LastReport = 0.0;
		if (FPlatformTime::Seconds() - LastReport > 2.0)
		{
			LastReport = FPlatformTime::Seconds();
			AddInfo(FString::Printf(TEXT("Min angle %.0f, view yaw %.0f:%s"), State->MinAngleBetweenEnemies(), State->Player->GetViewRotation().Yaw, *State->Describe()));
		}
		return State->MinAngleBetweenEnemies() >= 40.f;
	});
	WaitUntil(this, State, TEXT("The flanker stands outside the player's view cone (10 degrees tolerance)"), 15.0, [State]() { return State->FlankerOutOfView(10.f); });

	WaitUntil(this, State, TEXT("One enemy dies: roles and tokens are recomputed for three at once"), 1.0, [State, Settings]()
	{
		State->Kill(State->Enemies()[0]);
		return State->Enemies().Num() == 3 && State->CountRole(EEnemyRole::Pressure) == 1 && State->CountRole(EEnemyRole::Flank) == 1
			&& State->CountRole(EEnemyRole::Cover) == 1 && State->CountTokens() <= Settings->GetTokenCount(3);
	});

	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

#endif
