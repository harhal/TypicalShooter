#include "Loot/LootSettings.h"

namespace LootSettingsDefaults
{
	/** +5% per level: x1 at level 1, x5.95 at level 100. */
	constexpr float MultiplierAtMaxLevel = 1.f + 0.05f * (ULootSettings::MaxLevel - ULootSettings::MinLevel);
}

ULootSettings::ULootSettings()
{
	FRichCurve* Curve = LevelMultiplier.GetRichCurve();
	Curve->SetKeyInterpMode(Curve->AddKey(MinLevel, 1.f), RCIM_Linear);
	Curve->SetKeyInterpMode(Curve->AddKey(MaxLevel, LootSettingsDefaults::MultiplierAtMaxLevel), RCIM_Linear);
}

int32 ULootSettings::ClampLevel(const int32 Level)
{
	return FMath::Clamp(Level, MinLevel, MaxLevel);
}

float ULootSettings::GetLevelMultiplier(const int32 Level) const
{
	const FRichCurve* Curve = LevelMultiplier.GetRichCurveConst();
	if (!Curve || Curve->GetNumKeys() == 0)
	{
		return 1.f;
	}
	return Curve->Eval(static_cast<float>(ClampLevel(Level)));
}
