#include "Shooting/ShotPattern.h"

FVector ShotMath::AimDirection(const FVector& Muzzle, const FVector& AimPoint, const FVector& Fallback)
{
	const FVector ToAim = AimPoint - Muzzle;
	return ToAim.IsNearlyZero() ? Fallback.GetSafeNormal() : ToAim.GetSafeNormal();
}

TArray<FVector> ShotMath::ProjectileDirections(const FVector& AimDirection, const FShotPattern& Pattern, const float Scale)
{
	const FQuat Aim = AimDirection.GetSafeNormal().ToOrientationQuat();
	if (Pattern.Offsets.IsEmpty())
	{
		return { Aim.GetForwardVector() };
	}

	TArray<FVector> Directions;
	Directions.Reserve(Pattern.Offsets.Num());
	for (const FVector2D& Offset : Pattern.Offsets)
	{
		// Offset in the aim's local frame: yaw around its up axis, pitch around its right axis.
		const FQuat Local = FRotator(Offset.Y * Scale, Offset.X * Scale, 0.f).Quaternion();
		Directions.Add((Aim * Local).GetForwardVector());
	}
	return Directions;
}

FTransform ShotMath::SpawnTransform(const FVector& Muzzle, const FVector& Direction, const float StartDistance)
{
	const FVector Unit = Direction.GetSafeNormal();
	return FTransform(Unit.ToOrientationQuat(), Muzzle + Unit * StartDistance);
}
