// Test-only infinite gameplay effect without modifiers: power grid effect tests count its active instances.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "PowerGridTestEffect.generated.h"

UCLASS(NotBlueprintable, HideDropdown)
class UPowerGridTestEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UPowerGridTestEffect()
	{
		DurationPolicy = EGameplayEffectDurationType::Infinite;
	}
};
