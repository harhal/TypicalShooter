// Infinite effect adding item attack bonuses to the slot attributes (power grid R14a): one SetByCaller magnitude per
// slot and attack parameter, keyed by GetMagnitudeName. Unset magnitudes count as 0.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "Shooting/AttackTypes.h"
#include "AttackBonusEffect.generated.h"

UCLASS()
class TYPICALSHOOTER_API UAttackBonusEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UAttackBonusEffect();

	/** SetByCaller name of the bonus to Attribute of Slot. */
	static FName GetMagnitudeName(EAttackSlot Slot, EAttackAttribute Attribute);
};
