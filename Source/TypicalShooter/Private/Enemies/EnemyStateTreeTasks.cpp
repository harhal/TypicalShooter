#include "Enemies/EnemyStateTreeTasks.h"

#include "Enemies/EnemyCharacter.h"
#include "StateTreeExecutionContext.h"

namespace EnemyStateTreeTasks
{
	template <typename TInstanceData>
	AEnemyAIController* GetController(const FStateTreeExecutionContext& Context, const FStateTreeTaskCommonBase& Task)
	{
		return Cast<AEnemyAIController>(Context.GetInstanceData<TInstanceData>(Task).Controller);
	}
}

EStateTreeRunStatus FEnemyTrackTargetTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	return Tick(Context, 0.f);
}

EStateTreeRunStatus FEnemyTrackTargetTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	AEnemyAIController* Controller = EnemyStateTreeTasks::GetController<FInstanceDataType>(Context, *this);
	if (!Controller)
	{
		return EStateTreeRunStatus::Failed;
	}
	Controller->UpdateTarget();
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FEnemyShootTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	AEnemyAIController* Controller = EnemyStateTreeTasks::GetController<FInstanceDataType>(Context, *this);
	if (!Controller)
	{
		return EStateTreeRunStatus::Failed;
	}
	Controller->UpdateFire();
	return EStateTreeRunStatus::Running;
}

void FEnemyShootTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	AEnemyAIController* Controller = EnemyStateTreeTasks::GetController<FInstanceDataType>(Context, *this);
	if (AEnemyCharacter* Enemy = Controller ? Cast<AEnemyCharacter>(Controller->GetPawn()) : nullptr)
	{
		Enemy->SetFiring(false);
		Controller->ReleaseAttackToken(); // leaving combat frees the token for the group (R6)
	}
}

EStateTreeRunStatus FEnemyKeepDistanceTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	AEnemyAIController* Controller = EnemyStateTreeTasks::GetController<FInstanceDataType>(Context, *this);
	if (!Controller)
	{
		return EStateTreeRunStatus::Failed;
	}
	Controller->UpdatePosition(Context.GetInstanceData(*this).Settings);
	return EStateTreeRunStatus::Running;
}

void FEnemyKeepDistanceTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	if (AEnemyAIController* Controller = EnemyStateTreeTasks::GetController<FInstanceDataType>(Context, *this))
	{
		Controller->StopMovement();
	}
}
