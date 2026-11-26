#include "Loot/LootTable.h"

#include "Loot/LootGenerator.h"
#include "Loot/LootItemTemplate.h"
#include "Loot/LootRandom.h"
#include "Loot/LootSettings.h"

namespace LootTablePrivate
{
	TArray<float> TemplateWeights(const ULootTable& Table)
	{
		TArray<float> Weights;
		for (const FLootTemplateEntry& Entry : Table.Templates)
		{
			Weights.Add(Entry.Template ? Entry.Weight : 0.f);
		}
		return Weights;
	}

	TArray<float> RarityWeights(const ULootTable& Table, const int32 Level)
	{
		TArray<float> Weights;
		for (const FLootRarityEntry& Entry : Table.Rarities)
		{
			Weights.Add(Entry.Rarity ? Entry.GetWeight(Level) : 0.f);
		}
		return Weights;
	}
}

float FLootRarityEntry::GetWeight(const int32 Level) const
{
	const FRichCurve* Curve = WeightByLevel.GetRichCurveConst();
	return Curve && Curve->GetNumKeys() > 0 ? Curve->Eval(static_cast<float>(ULootSettings::ClampLevel(Level))) : 1.f;
}

TArray<FInventoryItemManifest> ULootTable::Roll(const int32 Level, const int32 Seed) const
{
	using namespace LootTablePrivate;
	FRandomStream Stream(Seed);
	const int32 ClampedLevel = ULootSettings::ClampLevel(Level);
	const TArray<float> Weights = TemplateWeights(*this);
	const TArray<float> RarityPicks = RarityWeights(*this, ClampedLevel);
	const int32 Count = ItemCount.Roll(Stream, GetDefault<ULootSettings>()->GetLevelMultiplier(ClampedLevel));

	TArray<FInventoryItemManifest> Items;
	for (int32 Index = 0; Index < Count; ++Index)
	{
		const int32 TemplateIndex = LootRandom::PickWeightedIndex(Stream, Weights);
		if (TemplateIndex == INDEX_NONE)
		{
			break;
		}
		const int32 RarityIndex = LootRandom::PickWeightedIndex(Stream, RarityPicks);
		FLootGenerationParams Params;
		Params.Level = ClampedLevel;
		Params.Seed = static_cast<int32>(Stream.GetUnsignedInt());
		Params.Rarity = RarityIndex != INDEX_NONE ? Rarities[RarityIndex].Rarity.Get() : nullptr;
		Items.Add(LootGenerator::Generate(*Templates[TemplateIndex].Template, Params));
	}
	return Items;
}
