#include "Shooting/ShotAim.h"

#include "Components/SceneComponent.h"
#include "Engine/HitResult.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

const FName FShotAim::MuzzleTag(TEXT("Muzzle"));

FShotAim FShotAim::Compute(const AActor& Avatar, const float Range)
{
	FVector EyeLocation;
	FRotator EyeRotation;
	Avatar.GetActorEyesViewPoint(EyeLocation, EyeRotation);

	FShotAim Aim;
	Aim.ViewDirection = EyeRotation.Vector();
	const USceneComponent* MuzzleComponent = Avatar.FindComponentByTag<USceneComponent>(MuzzleTag);
	Aim.Muzzle = MuzzleComponent ? MuzzleComponent->GetComponentLocation() : EyeLocation;

	const FVector TraceEnd = EyeLocation + Aim.ViewDirection * FMath::Max(Range, 1.f);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(ShotAim), true, &Avatar);
	FHitResult Hit;
	const UWorld* World = Avatar.GetWorld();
	const bool bHit = World && World->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, ECC_Visibility, Params);
	Aim.AimPoint = bHit ? Hit.ImpactPoint : TraceEnd;
	return Aim;
}
