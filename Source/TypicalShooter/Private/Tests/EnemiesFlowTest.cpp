// End-to-end enemy check in Play In Editor on Lvl_FirstPerson: the wave spawner brings enemies, their StateTree tracks
// the player, they keep the distance band and hurt the player; when the wave dies, the next one comes.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Enemies/EnemyAIController.h"
#include "Enemies/EnemyCharacter.h"
#include "Enemies/EnemyDirectorSubsystem.h"
#include "Enemies/EnemyWaveSpawner.h"
#include "Engine/Engine.h"
#include "Engine/HitResult.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "NavigationSystem.h"
#include "Shooting/HealthComponent.h"
#include "Shooting/ShootingTags.h"
#include "Shooting/ShotDamage.h"
#include "Shooting/ShotDamageReceiver.h"
#include "Tests/AutomationEditorCommon.h"

namespace EnemiesFlowTest
{
	struct FState
	{
		TWeakObjectPtr<UWorld> World;
		TWeakObjectPtr<APawn> Player;
		TWeakObjectPtr<AEnemyWaveSpawner> Spawner;
		TWeakObjectPtr<AEnemyCharacter> Watched;
		double StepStarted = 0.0;
		bool bFailed = false;
		int32 WavesBeforeKill = 0;

		bool TryResolve()
		{
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				if (Context.WorldType == EWorldType::PIE && Context.World())
				{
					World = Context.World();
				}
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

		TArray<AEnemyCharacter*> Enemies() const
		{
			const UEnemyDirectorSubsystem* Director = World.IsValid() ? World->GetSubsystem<UEnemyDirectorSubsystem>() : nullptr;
			return Director ? Director->GetEnemies() : TArray<AEnemyCharacter*>();
		}

		bool PlayerHurt() const
		{
			const UHealthComponent* Health = Player.IsValid() ? Player->FindComponentByClass<UHealthComponent>() : nullptr;
			return Health && (Health->GetHealth() < Health->GetMaxHealth() || Health->GetDefeats() > 0);
		}

		bool AnyEnemyInBand(const float Min, const float Max) const
		{
			for (const AEnemyCharacter* Enemy : Enemies())
			{
				const double Distance = FVector::Dist2D(Enemy->GetActorLocation(), Player->GetActorLocation());
				if (Distance >= Min && Distance <= Max)
				{
					return true;
				}
			}
			return false;
		}

		bool AllEnemiesTrackPlayer() const
		{
			const TArray<AEnemyCharacter*> Live = Enemies();
			for (const AEnemyCharacter* Enemy : Live)
			{
				const AEnemyAIController* Controller = Cast<AEnemyAIController>(Enemy->GetController());
				if (!Controller || Controller->GetTarget() != Player.Get())
				{
					return false;
				}
			}
			return Live.Num() > 0;
		}

		void KillAll() const
		{
			FShotDamage Damage;
			Damage.Amounts.Add(ShootingTags::Damage_Type_Physical, 100000.f);
			for (AEnemyCharacter* Enemy : Enemies())
			{
				IShotDamageReceiver::FindOnActor(Enemy)->ReceiveShotDamage(Damage, FHitResult());
			}
		}
	};

	/** Polls Condition every frame until it holds; error after Timeout seconds (later waits are skipped). */
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
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesFlowTest, "TypicalShooter.Enemies.Flow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FEnemiesFlowTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesFlowTest;
	const TSharedRef<FState> State = MakeShared<FState>();
	AEnemyWaveSpawner::SetWavesEnabled(true);
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));

	WaitUntil(this, State, TEXT("PIE has the player pawn and a wave spawner"), 15.0, [State]() { return State->TryResolve(); });
	WaitUntil(this, State, TEXT("The first wave spawned"), 15.0, [State]() { return State->Enemies().Num() > 0; });
	WaitUntil(this, State, TEXT("The enemies' StateTree tracks the player"), 5.0, [State]() { return State->AllEnemiesTrackPlayer(); });
	WaitUntil(this, State, TEXT("An enemy keeps within the distance band (800-1600, +-150)"), 20.0,
		[State]() { return State->AnyEnemyInBand(650.f, 1750.f); });
	WaitUntil(this, State, TEXT("Enemies hurt the player"), 20.0, [State]() { return State->PlayerHurt(); });
	WaitUntil(this, State, TEXT("The player is teleported 300 cm from an enemy"), 1.0, [State]()
	{
		const TArray<AEnemyCharacter*> Live = State->Enemies();
		if (Live.IsEmpty())
		{
			return false;
		}
		State->Watched = Live[0];
		const FVector Near = Live[0]->GetActorLocation() + FVector(300.f, 0.f, 0.f);
		return State->Player->TeleportTo(Near, State->Player->GetActorRotation());
	});
	WaitUntil(this, State, TEXT("The enemy backs off into the distance band over the navmesh"), 15.0, [this, State]()
	{
		static double LastReport = 0.0;
		if (State->Watched.IsValid() && FPlatformTime::Seconds() - LastReport > 2.0)
		{
			LastReport = FPlatformTime::Seconds();
			const UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(State->World.Get());
			FNavLocation Projected;
			const bool bOnNav = Nav && Nav->ProjectPointToNavigation(State->Watched->GetActorLocation(), Projected, FVector(100.f, 100.f, 300.f));
			const AEnemyAIController* Controller = Cast<AEnemyAIController>(State->Watched->GetController());
			AddInfo(FString::Printf(TEXT("Nav data: %s, enemy on navmesh: %d, at %s, stand point %s, move status %d"),
				Nav && Nav->GetDefaultNavDataInstance() ? *Nav->GetDefaultNavDataInstance()->GetName() : TEXT("none"), bOnNav,
				*State->Watched->GetActorLocation().ToCompactString(),
				Controller ? *Controller->GetStandPoint().ToCompactString() : TEXT("-"),
				Controller ? static_cast<int32>(Controller->GetMoveStatus()) : -1));
		}
		return State->Watched.IsValid()
			&& FVector::Dist2D(State->Watched->GetActorLocation(), State->Player->GetActorLocation()) >= 650.0;
	});
	WaitUntil(this, State, TEXT("The whole wave is killed"), 1.0, [State]()
	{
		State->WavesBeforeKill = State->Spawner->GetWavesStarted();
		State->KillAll();
		return State->Enemies().Num() == 0;
	});
	WaitUntil(this, State, TEXT("The next wave comes"), 15.0, [State]()
	{
		return State->Spawner->GetWavesStarted() > State->WavesBeforeKill && State->Enemies().Num() > 0;
	});

	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

#endif
