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

	/** Ammo kinds (R15): each has a reserve attribute in UAmmoAttributeSet. */
	TYPICALSHOOTER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ammo);
	TYPICALSHOOTER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ammo_Bullet);
	TYPICALSHOOTER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ammo_Battery);
	TYPICALSHOOTER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ammo_Grenade);
	TYPICALSHOOTER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ammo_Radiator);
	TYPICALSHOOTER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ammo_Particle);

	/** All concrete ammo kinds, in a fixed order. */
	TYPICALSHOOTER_API TConstArrayView<FGameplayTag> GetAmmoTypes();
}
