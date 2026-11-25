// Gameplay effects used by shooting tests (UCLASS can't be compiled out, so they exist in every build; tiny and unused elsewhere).

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "ShootingTestEffects.generated.h"

/** Infinite: +5 physical damage of the primary attack. */
UCLASS(NotBlueprintable, HideDropdown)
class UTestPrimaryDamageBonusEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UTestPrimaryDamageBonusEffect();
};
