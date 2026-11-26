// StateTree conditions of the enemy behaviour (ST_Enemy): which role the director gave the enemy. Used as enter
// conditions of the role states under Combat and on the transitions between them.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/Coordination/EnemyRole.h"
#include "StateTreeConditionBase.h"
#include "EnemyStateTreeConditions.generated.h"

class AAIController;

USTRUCT()
struct TYPICALSHOOTER_API FEnemyHasRoleConditionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AAIController> Controller;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	EEnemyRole Role = EEnemyRole::Pressure;
};

/** The director coordinates this enemy and gave it Role. */
USTRUCT(meta = (DisplayName = "Enemy: Has Role", Category = "Enemy"))
struct TYPICALSHOOTER_API FEnemyHasRoleCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FEnemyHasRoleConditionInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};

USTRUCT()
struct TYPICALSHOOTER_API FEnemyNotCoordinatedConditionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AAIController> Controller;
};

/** The director gives this enemy no role (it has no target). */
USTRUCT(meta = (DisplayName = "Enemy: Not Coordinated", Category = "Enemy"))
struct TYPICALSHOOTER_API FEnemyNotCoordinatedCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FEnemyNotCoordinatedConditionInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};
