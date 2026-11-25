#include "Shooting/AmmoDisplay.h"

#include "AbilitySystemComponent.h"
#include "Shooting/AmmoAttributeSet.h"
#include "Shooting/ShootingTags.h"

#define LOCTEXT_NAMESPACE "AmmoDisplay"

FText AmmoDisplay::GetReserveText(const UAbilitySystemComponent& Asc)
{
	TArray<FText> Lines;
	for (const FGameplayTag& AmmoType : ShootingTags::GetAmmoTypes())
	{
		const FGameplayAttribute Reserve = UAmmoAttributeSet::GetReserveAttribute(AmmoType);
		const int32 Amount = Reserve.IsValid() && Asc.HasAttributeSetForAttribute(Reserve)
			? FMath::FloorToInt32(Asc.GetNumericAttribute(Reserve)) : 0;
		if (Amount > 0)
		{
			FString Kind = AmmoType.ToString();
			Kind.Split(TEXT("."), nullptr, &Kind, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
			Lines.Add(FText::Format(LOCTEXT("Line", "{0}: {1}"), FText::FromString(Kind), FText::AsNumber(Amount)));
		}
	}
	return FText::Join(FText::FromString(TEXT("\n")), Lines);
}

#undef LOCTEXT_NAMESPACE
