// Project-wide enemy group coordination settings (Project Settings -> Game -> Enemy Coordination).

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Enemies/Coordination/EnemyRole.h"
#include "Enemies/Coordination/EnemySectors.h"
#include "Enemies/Coordination/EnemyTokenCount.h"
#include "Enemies/Coordination/EnemyTokenPool.h"
#include "EnemyCoordinationSettings.generated.h"

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Enemy Coordination"))
class TYPICALSHOOTER_API UEnemyCoordinationSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return TEXT("Game"); }

	/** Attack tokens for a group of GroupSize enemies. */
	int32 GetTokenCount(int32 GroupSize) const;

	/** Groups are recomputed this often (and at once on register / death / target change). */
	UPROPERTY(Config, EditAnywhere, Category = "Groups", meta = (ClampMin = "0.05", Units = "s"))
	float UpdateInterval = 0.5f;

	/** How many enemies of one target group may attack at once. */
	UPROPERTY(Config, EditAnywhere, Category = "Tokens", meta = (ShowOnlyInnerProperties))
	FEnemyTokenCountSettings Tokens;

	/** How long a token is held and when it passes on. */
	UPROPERTY(Config, EditAnywhere, Category = "Tokens")
	FEnemyTokenRotationSettings TokenRotation;

	/** Wanted share of each role in a target group. */
	UPROPERTY(Config, EditAnywhere, Category = "Roles")
	FEnemyRoleWeights RoleWeights;

	/** Sectors around the target; flankers stay out of its view. */
	UPROPERTY(Config, EditAnywhere, Category = "Sectors", meta = (ShowOnlyInnerProperties))
	FEnemySectorSettings Sectors;
};
