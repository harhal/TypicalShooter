// Damage carried by one projectile hit: an amount per damage type tag.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ShotDamage.generated.h"

USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FShotDamage
{
	GENERATED_BODY()

	/** Damage per type (keys under Damage.Type). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (Categories = "Damage.Type"))
	TMap<FGameplayTag, float> Amounts;

	/** Who fired the shot (may be null). */
	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	TWeakObjectPtr<AActor> Instigator;

	/** Amount of one damage type, 0 if absent. */
	float Get(const FGameplayTag& Type) const;

	/** Sum over all types. */
	float GetTotal() const;

	/** True if any type deals positive damage. */
	bool HasDamage() const;

	/** Multiplies every amount by Factor (clamped to >= 0). */
	void Scale(float Factor);
};
