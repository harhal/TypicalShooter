#include "Shooting/ShotProjectileMovementComponent.h"

#include "Components/PrimitiveComponent.h"
#include "Shooting/ShotProjectile.h"

UProjectileMovementComponent::EHandleBlockingHitResult UShotProjectileMovementComponent::HandleBlockingHit(
	const FHitResult& Hit, const float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)
{
	AShotProjectile* Projectile = Cast<AShotProjectile>(GetOwner());
	if (!Projectile || !UpdatedPrimitive || !Projectile->HandleShotHit(Hit))
	{
		return Super::HandleBlockingHit(Hit, TimeTick, MoveDelta, SubTickTimeRemaining);
	}

	// Pierce: never collide with this actor (or component) again and keep moving.
	if (AActor* HitActor = Hit.GetActor())
	{
		UpdatedPrimitive->IgnoreActorWhenMoving(HitActor, true);
	}
	else if (UPrimitiveComponent* HitComponent = Hit.GetComponent())
	{
		UpdatedPrimitive->IgnoreComponentWhenMoving(HitComponent, true);
	}
	return EHandleBlockingHitResult::AdvanceNextSubstep;
}
