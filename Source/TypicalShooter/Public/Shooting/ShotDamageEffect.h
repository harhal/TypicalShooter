// Instant effect: adds SetByCaller damage of every type (keyed by Damage.Type.* tags) to IncomingDamage.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "ShotDamageEffect.generated.h"

UCLASS()
class TYPICALSHOOTER_API UShotDamageEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UShotDamageEffect();
};
