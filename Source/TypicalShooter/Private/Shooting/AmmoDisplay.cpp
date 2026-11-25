#include "Shooting/AmmoDisplay.h"

#include "AbilitySystemComponent.h"
#include "Shooting/AmmoAttributeSet.h"
#include "Shooting/AttackAttributeSet.h"
#include "Shooting/ReloadGameplayAbility.h"
#include "Shooting/ShootingTags.h"

#define LOCTEXT_NAMESPACE "AmmoDisplay"

FText AmmoDisplay::GetSlotText(const UAbilitySystemComponent& Asc, const EAttackSlot Slot)
{
	const int32 Clip = FMath::FloorToInt32(Asc.GetNumericAttribute(UAttackAttributeSet::GetAttribute(Slot, EAttackAttribute::Ammo)));

	const UReloadGameplayAbility* Reload = UReloadGameplayAbility::FindForSlot(Asc, Slot);
	const FGameplayAttribute Reserve = Reload ? UAmmoAttributeSet::GetReserveAttribute(Reload->AmmoType) : FGameplayAttribute();
	const FText ReserveText = Reserve.IsValid() && Asc.HasAttributeSetForAttribute(Reserve)
		? FText::AsNumber(FMath::FloorToInt32(Asc.GetNumericAttribute(Reserve)))
		: FText::FromString(TEXT("∞"));

	const FText Counter = FText::Format(LOCTEXT("Counter", "{0} / {1}"), FText::AsNumber(Clip), ReserveText);
	if (Asc.HasMatchingGameplayTag(ShootingTags::GetReloadingTag(static_cast<uint8>(Slot))))
	{
		return FText::Format(LOCTEXT("Reloading", "{0}  Reloading..."), Counter);
	}
	return Counter;
}

#undef LOCTEXT_NAMESPACE
