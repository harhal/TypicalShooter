// Text of the HUD health counter.

#pragma once

#include "CoreMinimal.h"

class UAbilitySystemComponent;

namespace HealthDisplay
{
	/** "Health: 73 / 100" (current rounded up); empty without a health attribute set. */
	TYPICALSHOOTER_API FText GetHealthText(const UAbilitySystemComponent& Asc);
}
