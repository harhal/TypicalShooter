// Text of the HUD ammo counter: the character's reserve per ammo kind (attacks have no clips, power grid R9a).

#pragma once

#include "CoreMinimal.h"

class UAbilitySystemComponent;

namespace AmmoDisplay
{
	/** "Bullet: 120\nParticle: 24": one line per ammo kind with a non-empty reserve, in ShootingTags::GetAmmoTypes() order. */
	TYPICALSHOOTER_API FText GetReserveText(const UAbilitySystemComponent& Asc);
}
