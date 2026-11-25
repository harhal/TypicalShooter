#include "Shooting/ShootingTags.h"

namespace ShootingTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Damage_Type, "Damage.Type", "Parent of all damage types");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Damage_Type_Physical, "Damage.Type.Physical", "Physical damage");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Damage_Type_Acid, "Damage.Type.Acid", "Acid damage");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Damage_Type_Electric, "Damage.Type.Electric", "Electric damage");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Damage_Type_Plasma, "Damage.Type.Plasma", "Plasma damage");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Damage_Type_Ice, "Damage.Type.Ice", "Ice damage");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Damage_Type_Radiation, "Damage.Type.Radiation", "Radiation damage");

	TConstArrayView<FGameplayTag> GetDamageTypes()
	{
		static const TArray<FGameplayTag> Types = {
			Damage_Type_Physical, Damage_Type_Acid, Damage_Type_Electric,
			Damage_Type_Plasma, Damage_Type_Ice, Damage_Type_Radiation };
		return Types;
	}
}
