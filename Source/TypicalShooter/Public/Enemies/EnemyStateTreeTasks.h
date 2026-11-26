// StateTree tasks of the enemy behaviour (ST_Enemy, AI component schema). Thin wrappers: each tick calls one step of
// AEnemyAIController. All run until their state exits; put them in one state to run in parallel.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/EnemyAIController.h"
#include "StateTreeTaskBase.h"
#include "EnemyStateTreeTasks.generated.h"

class AAIController;

USTRUCT()
struct TYPICALSHOOTER_API FEnemyControllerTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AAIController> Controller;
};

/** Target = nearest player pawn (director), look at it. */
USTRUCT(meta = (DisplayName = "Enemy: Track Target", Category = "Enemy"))
struct TYPICALSHOOTER_API FEnemyTrackTargetTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FEnemyControllerTaskInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, float DeltaTime) const override;
};

/** Fire while the target is in range and visible; stop on exit. */
USTRUCT(meta = (DisplayName = "Enemy: Shoot When Visible", Category = "Enemy"))
struct TYPICALSHOOTER_API FEnemyShootTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FEnemyControllerTaskInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};

USTRUCT()
struct TYPICALSHOOTER_API FEnemyKeepDistanceInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AAIController> Controller;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	FEnemyStandSettings Settings;
};

/** Keep within the distance band of the target (navmesh moves); stop moving on exit. */
USTRUCT(meta = (DisplayName = "Enemy: Keep Distance", Category = "Enemy"))
struct TYPICALSHOOTER_API FEnemyKeepDistanceTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FEnemyKeepDistanceInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
