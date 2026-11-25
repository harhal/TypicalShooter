// Projectile movement that lets AShotProjectile decide whether to pierce what it hit.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "ShotProjectileMovementComponent.generated.h"

UCLASS()
class TYPICALSHOOTER_API UShotProjectileMovementComponent : public UProjectileMovementComponent
{
	GENERATED_BODY()

protected:
	/** Pierced hits ignore the hit actor from now on and continue the move; others stop as usual. */
	virtual EHandleBlockingHitResult HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta,
		float& SubTickTimeRemaining) override;
};
