#include "Shooting/HealthDisplay.h"

#include "AbilitySystemComponent.h"
#include "Shooting/HealthAttributeSet.h"

FText HealthDisplay::GetHealthText(const UAbilitySystemComponent& Asc)
{
	if (!Asc.HasAttributeSetForAttribute(UHealthAttributeSet::GetHealthAttribute()))
	{
		return FText::GetEmpty();
	}
	const float Health = FMath::CeilToFloat(Asc.GetNumericAttribute(UHealthAttributeSet::GetHealthAttribute()));
	const float Max = Asc.GetNumericAttribute(UHealthAttributeSet::GetMaxHealthAttribute());
	return FText::FromString(FString::Printf(TEXT("Health: %.0f / %.0f"), Health, Max));
}
