// Native gameplay tags used by the shooting feature.

#pragma once

#include "NativeGameplayTags.h"

namespace ShootingTags
{
	/** Damage types: keys of FShotDamage::Amounts. */
	TYPICALSHOOTER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Damage_Type);
	TYPICALSHOOTER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Damage_Type_Physical);
	TYPICALSHOOTER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Damage_Type_Acid);
	TYPICALSHOOTER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Damage_Type_Electric);
	TYPICALSHOOTER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Damage_Type_Plasma);
	TYPICALSHOOTER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Damage_Type_Ice);
	TYPICALSHOOTER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Damage_Type_Radiation);

	/** All concrete damage types, in a fixed order. */
	TYPICALSHOOTER_API TConstArrayView<FGameplayTag> GetDamageTypes();

	/** Owned (loose, server) while an attack slot reloads; blocks that slot's attack. */
	TYPICALSHOOTER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Reloading_Primary);
	TYPICALSHOOTER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Reloading_Alternate);

	/** State_Reloading_* of a slot (0 = primary, 1 = alternate). */
	TYPICALSHOOTER_API FGameplayTag GetReloadingTag(uint8 SlotIndex);
}
