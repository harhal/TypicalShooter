#include "Shooting/ShotDamage.h"

float FShotDamage::Get(const FGameplayTag& Type) const
{
	const float* Amount = Amounts.Find(Type);
	return Amount ? *Amount : 0.f;
}

float FShotDamage::GetTotal() const
{
	float Total = 0.f;
	for (const TPair<FGameplayTag, float>& Pair : Amounts)
	{
		Total += Pair.Value;
	}
	return Total;
}

bool FShotDamage::HasDamage() const
{
	for (const TPair<FGameplayTag, float>& Pair : Amounts)
	{
		if (Pair.Value > 0.f)
		{
			return true;
		}
	}
	return false;
}

void FShotDamage::Scale(const float Factor)
{
	const float SafeFactor = FMath::Max(0.f, Factor);
	for (TPair<FGameplayTag, float>& Pair : Amounts)
	{
		Pair.Value *= SafeFactor;
	}
}
