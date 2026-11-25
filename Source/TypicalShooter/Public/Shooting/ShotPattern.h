// Shot pattern (one projectile per point) and the pure math of a shot: aim from the muzzle,
// projectile directions, projectile spawn transforms.

#pragma once

#include "CoreMinimal.h"
#include "ShotPattern.generated.h"

/** Angular offsets of the projectiles of one shot relative to the aim direction; one projectile per point. */
USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FShotPattern
{
	GENERATED_BODY()

	/** X = yaw to the right, Y = pitch up, in degrees (multiplied by the attack's pattern scale). Empty = one centred projectile. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot")
	TArray<FVector2D> Offsets = { FVector2D::ZeroVector };

	int32 GetNumProjectiles() const { return FMath::Max(Offsets.Num(), 1); }
};

namespace ShotMath
{
	/** Unit direction from the muzzle to the aim point; Fallback (normalized) if they coincide. */
	TYPICALSHOOTER_API FVector AimDirection(const FVector& Muzzle, const FVector& AimPoint, const FVector& Fallback);

	/** One unit direction per pattern point: AimDirection rotated by the point's offset * Scale. */
	TYPICALSHOOTER_API TArray<FVector> ProjectileDirections(const FVector& AimDirection, const FShotPattern& Pattern, float Scale);

	/** Projectile spawn: StartDistance from the muzzle along Direction, facing Direction. */
	TYPICALSHOOTER_API FTransform SpawnTransform(const FVector& Muzzle, const FVector& Direction, float StartDistance);
}
