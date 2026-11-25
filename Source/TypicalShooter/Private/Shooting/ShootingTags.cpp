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

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Reloading_Primary, "State.Reloading.Primary", "Primary attack is reloading");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Reloading_Alternate, "State.Reloading.Alternate", "Alternate attack is reloading");

	FGameplayTag GetReloadingTag(const uint8 SlotIndex)
	{
		return SlotIndex == 0 ? State_Reloading_Primary.GetTag() : State_Reloading_Alternate.GetTag();
	}
}
