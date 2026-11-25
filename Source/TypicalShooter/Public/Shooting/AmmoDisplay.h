// Text of the HUD ammo counter: "clip / reserve" of an attack slot, plus a reload marker.

#pragma once

#include "CoreMinimal.h"
#include "Shooting/AttackTypes.h"

class UAbilitySystemComponent;

namespace AmmoDisplay
{
	/** "7 / 40" (reserve of the slot reload's ammo kind; "∞" when unlimited), "7 / 40  Reloading..." while reloading. */
	TYPICALSHOOTER_API FText GetSlotText(const UAbilitySystemComponent& Asc, EAttackSlot Slot);
}
