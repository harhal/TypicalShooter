#include "Enemies/EnemyAIController.h"

#include "Components/StateTreeAIComponent.h"
#include "Enemies/Coordination/EnemyCoordinationSettings.h"
#include "Enemies/Coordination/EnemyRoleStand.h"
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
		// The group's attack token first: an enemy without it holds fire even when it sees the target.
		Enemy->SetFiring(HasAttackToken() && Enemy->CanAttack(Target));
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
	ApplyCoordination(Query);
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

bool AEnemyAIController::GetCoordinatedRole(EEnemyRole& OutRole) const
{
	const UEnemyDirectorSubsystem* Director = GetWorld()->GetSubsystem<UEnemyDirectorSubsystem>();
	const FEnemyAssignment* Assignment = Director && GetEnemy() ? Director->GetAssignment(GetEnemy()) : nullptr;
	if (Assignment)
	{
		OutRole = Assignment->Role;
	}
	return Assignment != nullptr;
}

TArray<FName> AEnemyAIController::GetActiveStateNames() const
{
	return StateTreeAI->GetActiveStateNames();
}

bool AEnemyAIController::HasAttackToken() const
{
	const UEnemyDirectorSubsystem* Director = GetWorld()->GetSubsystem<UEnemyDirectorSubsystem>();
	return Director && Director->HasAttackToken(GetEnemy());
}

void AEnemyAIController::ReleaseAttackToken()
{
	// The StateTree stops (and the shoot task exits) also while garbage collection destroys this controller and its
	// world: the director is gone then, and so are the tokens.
	if (IsUnreachable() || HasAnyFlags(RF_BeginDestroyed | RF_FinishDestroyed))
	{
		return;
	}
	const UWorld* World = GetWorld();
	if (!World || World->bIsTearingDown)
	{
		return;
	}
	if (UEnemyDirectorSubsystem* Director = World->GetSubsystem<UEnemyDirectorSubsystem>())
	{
		Director->ReleaseAttackToken(GetEnemy());
	}
}

void AEnemyAIController::ApplyCoordination(FEnemyStandQuery& Query) const
{
	const UEnemyDirectorSubsystem* Director = GetWorld()->GetSubsystem<UEnemyDirectorSubsystem>();
	const AEnemyCharacter* Enemy = GetEnemy();
	const FEnemyAssignment* Assignment = Director && Enemy ? Director->GetAssignment(Enemy) : nullptr;
	if (!Assignment)
	{
		return; // not coordinated: the task's own band, the enemy's own side
	}
	EnemyRoleStand::FInput Input;
	Input.Role = Assignment->Role;
	Input.bHasToken = Director->HasAttackToken(Enemy);
	Input.Band = Enemy->GetRoleBand(Assignment->Role);
	Input.SectorYaw = Assignment->SectorYaw;
	Input.SectorHalfWidth = Assignment->SectorHalfWidth;
	Input.MinSeparation = GetDefault<UEnemyCoordinationSettings>()->Sectors.MinSeparation;
	EnemyRoleStand::Apply(Query, Input);
}

void AEnemyAIController::StopCombat()
{
	ReleaseAttackToken();
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
