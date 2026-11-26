// Project-wide enemy group coordination settings (Project Settings -> Game -> Enemy Coordination).

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Enemies/Coordination/EnemyRole.h"
#include "Enemies/Coordination/EnemyTokenCount.h"
#include "EnemyCoordinationSettings.generated.h"

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Enemy Coordination"))
class TYPICALSHOOTER_API UEnemyCoordinationSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return TEXT("Game"); }

	/** Attack tokens for a group of GroupSize enemies. */
	int32 GetTokenCount(int32 GroupSize) const;

	/** How many enemies of one target group may attack at once. */
	UPROPERTY(Config, EditAnywhere, Category = "Tokens", meta = (ShowOnlyInnerProperties))
	FEnemyTokenCountSettings Tokens;

	/** Wanted share of each role in a target group. */
	UPROPERTY(Config, EditAnywhere, Category = "Roles")
	FEnemyRoleWeights RoleWeights;
};
