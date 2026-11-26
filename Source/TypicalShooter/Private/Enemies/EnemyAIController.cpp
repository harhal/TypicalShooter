#include "Enemies/EnemyAIController.h"

#include "Components/StateTreeAIComponent.h"
#include "Enemies/EnemyCharacter.h"
#include "Enemies/EnemyDirectorSubsystem.h"
#include "Enemies/EnemyPositioning.h"
#include "Engine/World.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"

AEnemyAIController::AEnemyAIController()
{
	StateTreeAI = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeAI"));
	StateTreeAI->SetStartLogicAutomatically(false);
	Random.GenerateNewSeed();
}

void AEnemyAIController::UpdateTarget()
{
	const UEnemyDirectorSubsystem* Director = GetWorld()->GetSubsystem<UEnemyDirectorSubsystem>();
	AActor* NewTarget = Director ? Director->FindTargetFor(GetPawn()) : nullptr;
	if (NewTarget != Target || GetFocusActor() != NewTarget)
	{
		Target = NewTarget;
		if (Target)
		{
			SetFocus(Target);
		}
		else
		{
			ClearFocus(EAIFocusPriority::Gameplay);
		}
	}
}

void AEnemyAIController::UpdateFire()
{
	if (AEnemyCharacter* Enemy = GetEnemy())
	{
		Enemy->UpdateFiring(Target);
	}
}

bool AEnemyAIController::UpdatePosition(const FEnemyStandSettings& Settings)
{
	const AEnemyCharacter* Enemy = GetEnemy();
	if (!Enemy || !Target)
	{
		return false;
	}
	const double Now = GetWorld()->GetTimeSeconds();
	const bool bMoving = GetMoveStatus() == EPathFollowingStatus::Moving;
	if (bMoving && Now - LastStandChoiceTime < Settings.RepathInterval)
	{
		return false;
	}

	FEnemyStandQuery Query;
	Query.Self = Enemy->GetActorLocation();
	Query.Target = Target->GetActorLocation();
	Query.MinDistance = Settings.MinDistance;
	Query.MaxDistance = FMath::Max(Settings.MinDistance, Settings.MaxDistance);
	Query.JitterDegrees = Settings.JitterDegrees;
	Query.FlankDegrees = Settings.FlankDegrees;
	Query.bHasLineOfSight = Enemy->CanShootAt(Target);
	LastStandChoiceTime = Now;
	if (!EnemyPositioning::ChooseReachableStandPoint(Query, Random, [this](FVector& Point) { return ProjectToNavigation(Point); }, StandPoint))
	{
		if (bMoving)
		{
			StopMovement(); // reached the band on the way
		}
		return false;
	}
	MoveToStandPoint(Settings);
	return true;
}

bool AEnemyAIController::ProjectToNavigation(FVector& Point) const
{
	const UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!Nav || !Nav->GetDefaultNavDataInstance())
	{
		return true; // no nav data (tests): straight-line moves, any point goes
	}
	// Small horizontal extent: a point outside the arena must not snap back to its edge.
	FNavLocation Projected;
	if (!Nav->ProjectPointToNavigation(Point, Projected, FVector(50.f, 50.f, 300.f)))
	{
		return false;
	}
	Point = Projected.Location;
	return true;
}

void AEnemyAIController::MoveToStandPoint(const FEnemyStandSettings& Settings)
{
	// Navmesh path when there is one; straight line otherwise (no navigation data, tests).
	const EPathFollowingRequestResult::Type Result =
		MoveToLocation(StandPoint, Settings.AcceptanceRadius, true, true, true, true);
	if (Result == EPathFollowingRequestResult::Failed)
	{
		MoveToLocation(StandPoint, Settings.AcceptanceRadius, true, false, false, true);
	}
}

void AEnemyAIController::StopCombat()
{
	if (AEnemyCharacter* Enemy = GetEnemy())
	{
		Enemy->SetFiring(false);
	}
	StopMovement();
	ClearFocus(EAIFocusPriority::Gameplay);
	Target = nullptr;
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (Behavior)
	{
		StateTreeAI->SetStateTree(Behavior);
		StateTreeAI->StartLogic();
	}
}

void AEnemyAIController::OnUnPossess()
{
	StateTreeAI->StopLogic(TEXT("Unpossessed"));
	Super::OnUnPossess();
}

AEnemyCharacter* AEnemyAIController::GetEnemy() const
{
	return Cast<AEnemyCharacter>(GetPawn());
}
