#include "Loot/LootGenerator.h"

#include "Loot/LootItemTemplate.h"
#include "Loot/LootOriginFragment.h"
#include "Loot/LootRandom.h"
#include "Loot/LootSettings.h"
#include "PowerGrid/PowerGridFragment.h"

namespace LootGeneratorPrivate
{
	int32 RollOpenFaces(const ULootItemTemplate& Template, FRandomStream& Stream)
	{
		TArray<float> Weights;
		for (const FLootFaceShape& Shape : Template.FaceShapes)
		{
			Weights.Add(Shape.Weight);
		}
		const int32 Picked = LootRandom::PickWeightedIndex(Stream, Weights);
		const int32 Mask = Picked != INDEX_NONE ? Template.FaceShapes[Picked].OpenFaces : HexFaces::AllFaces;
		return Template.bRandomRotation ? HexFaces::RotateMask(Mask, Stream.RandRange(0, HexFaces::Num - 1)) : Mask;
	}

	FPowerGrid_ItemFragment MakePowerGrid(const ULootItemTemplate& Template, FRandomStream& Stream, const float LevelMultiplier)
	{
		FPowerGrid_ItemFragment PowerGrid;
		PowerGrid.ItemType = Template.ItemType;
		PowerGrid.OpenFaces = RollOpenFaces(Template, Stream);
		if (Template.ItemType == EPowerItemType::Source)
		{
			PowerGrid.Source = Template.Source.Roll(Stream, LevelMultiplier);
		}
		else if (Template.ItemType == EPowerItemType::Receiver)
		{
			PowerGrid.Receiver = Template.Receiver.Roll(Stream, LevelMultiplier);
		}
		return PowerGrid;
	}

	FLootOrigin_ItemFragment MakeOrigin(const ULootItemTemplate& Template, const FLootGenerationParams& Params)
	{
		FLootOrigin_ItemFragment Origin;
		Origin.Template = &Template;
		Origin.Level = ULootSettings::ClampLevel(Params.Level);
		Origin.Seed = Params.Seed;
		return Origin;
	}
}

FInventoryItemManifest LootGenerator::Generate(const ULootItemTemplate& Template, const FLootGenerationParams& Params)
{
	using namespace LootGeneratorPrivate;
	FRandomStream Stream(Params.Seed);
	const float LevelMultiplier = GetDefault<ULootSettings>()->GetLevelMultiplier(Params.Level);

	FInventoryItemManifest Manifest;
	Manifest.Fragments = Template.FixedFragments;
	Manifest.Fragments.Add(TInstancedStruct<FItemFragment>::Make<FPowerGrid_ItemFragment>(MakePowerGrid(Template, Stream, LevelMultiplier)));
	Manifest.Fragments.Add(TInstancedStruct<FItemFragment>::Make<FLootOrigin_ItemFragment>(MakeOrigin(Template, Params)));
	return Manifest;
}
