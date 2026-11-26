#include "Enemies/EnemyStateTreeConditions.h"

#include "Enemies/EnemyAIController.h"
#include "StateTreeExecutionContext.h"

bool FEnemyHasRoleCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);
	const AEnemyAIController* Controller = Cast<AEnemyAIController>(Data.Controller);
	EEnemyRole Role;
	return Controller && Controller->GetCoordinatedRole(Role) && Role == Data.Role;
}

bool FEnemyNotCoordinatedCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);
	const AEnemyAIController* Controller = Cast<AEnemyAIController>(Data.Controller);
	EEnemyRole Role;
	return !Controller || !Controller->GetCoordinatedRole(Role);
}
