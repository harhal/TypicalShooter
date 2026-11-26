// How a role shapes where an enemy stands (R10): distance bands per role, and what the enemy does while it waits for an
// attack token. Pure logic on top of FEnemyStandQuery.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/Coordination/EnemyRole.h"
#include "Enemies/EnemyPositioning.h"
#include "EnemyRoleStand.generated.h"

/** Horizontal distance band [Min, Max] from the target. */
USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FEnemyDistanceBand
{
	GENERATED_BODY()

	FEnemyDistanceBand() = default;
	FEnemyDistanceBand(const float InMin, const float InMax) : Min(InMin), Max(InMax) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance", meta = (ClampMin = "0", Units = "cm"))
	float Min = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance", meta = (ClampMin = "0", Units = "cm"))
	float Max = 1600.f;
};

USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FEnemyRoleBands
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Roles")
	FEnemyDistanceBand Pressure = {500.f, 900.f};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Roles")
	FEnemyDistanceBand Flank = {700.f, 1200.f};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Roles")
	FEnemyDistanceBand Cover = {1400.f, 2000.f};

	const FEnemyDistanceBand& Get(EEnemyRole Role) const;
};

namespace EnemyRoleStand
{
	struct FInput
	{
		EEnemyRole Role = EEnemyRole::Pressure;
		bool bHasToken = false;
		/** The enemy's band for its role (its class may override the project's). */
		FEnemyDistanceBand Band;
		float SectorYaw = 0.f;
		float SectorHalfWidth = 180.f;
		float MinSeparation = 0.f;
	};

	/**
	 * Sets band and sector on Query. Pressure without a token keeps to the near half of its band (closes in); Cover
	 * without a token holds anywhere in its band while it sees the target; Flank always heads for its sector.
	 */
	TYPICALSHOOTER_API void Apply(FEnemyStandQuery& Query, const FInput& Input);
}
