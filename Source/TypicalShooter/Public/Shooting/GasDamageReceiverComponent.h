// Damage receiver for actors with an ability system: applies the shot damage effect to the owner's ASC.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Shooting/ShotDamageReceiver.h"
#include "GasDamageReceiverComponent.generated.h"

class UGameplayEffect;

UCLASS(ClassGroup = (Shooting), meta = (BlueprintSpawnableComponent))
class TYPICALSHOOTER_API UGasDamageReceiverComponent : public UActorComponent, public IShotDamageReceiver
{
	GENERATED_BODY()

public:
	UGasDamageReceiverComponent();

	/** Effect applied per hit; gets each damage type as a SetByCaller magnitude under its Damage.Type.* tag. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	TSubclassOf<UGameplayEffect> DamageEffect;

	virtual void ReceiveShotDamage(const FShotDamage& Damage, const FHitResult& Hit) override;
};
