// Where around its target each enemy of a group stands: N evenly spaced slots (yaw around the target), phased to fit
// the enemies' current angles so a spread group stays put; flankers get slots outside the target's view cone, everyone
// else the nearest free slot. Pure math, angles in degrees.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/Coordination/EnemyRole.h"
#include "EnemySectors.generated.h"

USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FEnemySectorSettings
{
	GENERATED_BODY()

	/** Flankers stand further than this from the target's horizontal view direction. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sectors", meta = (ClampMin = "0", ClampMax = "180"))
	float FlankViewHalfAngle = 60.f;

	/** Flank sectors follow the target's view only after it turned this far since the last time (hysteresis). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sectors", meta = (ClampMin = "0", ClampMax = "180"))
	float FlankReaimDegrees = 30.f;

	/** Stand points of neighbouring sectors keep at least this far apart, while the sectors are wide enough. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sectors", meta = (ClampMin = "0", Units = "cm"))
	float MinSeparation = 250.f;
};

struct FEnemySectorMember
{
	EEnemyRole Role = EEnemyRole::Pressure;
	/** Current yaw of the enemy around the target (see YawAround). */
	float CurrentYaw = 0.f;
};

struct FEnemySectorQuery
{
	TArray<FEnemySectorMember> Members;
	/** Horizontal view yaw of the target the flank sectors are based on. */
	float ViewYaw = 0.f;
	float FlankViewHalfAngle = 60.f;
};

namespace EnemySectors
{
	/** Sector yaw (-180..180] per member, same order. */
	TYPICALSHOOTER_API TArray<float> Assign(const FEnemySectorQuery& Query);

	/** Half of one sector's width for a group of GroupSize (180 / N). */
	TYPICALSHOOTER_API float SectorHalfWidth(int32 GroupSize);

	/** Yaw of Point seen from Target, horizontally. */
	TYPICALSHOOTER_API float YawAround(const FVector& Target, const FVector& Point);

	/** The target's view turned more than Threshold from the yaw the flank sectors were last based on. */
	TYPICALSHOOTER_API bool ShouldReaimFlank(float ReferenceViewYaw, float ViewYaw, float Threshold);
}
