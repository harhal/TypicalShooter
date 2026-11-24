// Interface for components that take damage from shots (GAS actors, breakable walls, ...).

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ShotDamageReceiver.generated.h"

struct FHitResult;
struct FShotDamage;

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UShotDamageReceiver : public UInterface
{
	GENERATED_BODY()
};

class TYPICALSHOOTER_API IShotDamageReceiver
{
	GENERATED_BODY()

public:
	/** Applies Damage from a hit. Server only. */
	virtual void ReceiveShotDamage(const FShotDamage& Damage, const FHitResult& Hit) = 0;

	/** First component of Actor implementing this interface, or nullptr. */
	static IShotDamageReceiver* FindOnActor(const AActor* Actor);
};
