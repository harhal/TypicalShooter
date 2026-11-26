#include "Loot/LootGenerator.h"

#include "Loot/LootItemTemplate.h"
#include "Loot/LootItemText.h"
#include "Loot/LootModifier.h"
#include "Loot/LootModifierPicker.h"
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

	/** Rolled steps sorted by threshold; character effects get the item level. */
	TArray<FLootRolledModifier> RollModifiers(TConstArrayView<const ULootModifier*> Modifiers, FRandomStream& Stream, const float LevelMultiplier, const int32 Level)
	{
		TArray<FLootRolledModifier> Rolled;
		for (const ULootModifier* Modifier : Modifiers)
		{
			FLootRolledModifier& Entry = Rolled.Add_GetRef({Modifier, Modifier->Roll(Stream, LevelMultiplier)});
			Entry.Step.CharacterEffectLevel = Level;
		}
		Rolled.StableSort([](const FLootRolledModifier& A, const FLootRolledModifier& B) { return A.Step.Threshold < B.Step.Threshold; });
		return Rolled;
	}

	FPowerEffects_ItemFragment MakeEffects(TConstArrayView<FLootRolledModifier> Rolled)
	{
		FPowerEffects_ItemFragment Effects;
		for (const FLootRolledModifier& Entry : Rolled)
		{
			Effects.Effects.Add(Entry.Step);
		}
		return Effects;
	}

	template<typename T>
	void AddFragment(FInventoryItemManifest& Manifest, const T& Fragment)
	{
		Manifest.Fragments.Add(TInstancedStruct<FItemFragment>::Make<T>(Fragment));
	}

	FLootOrigin_ItemFragment MakeOrigin(const ULootItemTemplate& Template, const FLootGenerationParams& Params, TConstArrayView<const ULootModifier*> Modifiers)
	{
		FLootOrigin_ItemFragment Origin;
		Origin.Template = &Template;
		Origin.Level = ULootSettings::ClampLevel(Params.Level);
		Origin.Seed = Params.Seed;
		Origin.Rarity = Params.Rarity;
		Origin.Modifiers = Modifiers;
		return Origin;
	}
}

FInventoryItemManifest LootGenerator::Generate(const ULootItemTemplate& Template, const FLootGenerationParams& Params)
{
	using namespace LootGeneratorPrivate;
	FRandomStream Stream(Params.Seed);
	const int32 Level = ULootSettings::ClampLevel(Params.Level);
	const float LevelMultiplier = GetDefault<ULootSettings>()->GetLevelMultiplier(Level);

	const FPowerGrid_ItemFragment PowerGrid = MakePowerGrid(Template, Stream, LevelMultiplier);
	const TArray<const ULootModifier*> Modifiers = LootModifierPicker::Pick(Template.ModifierPool, Params.Rarity, Stream);
	const TArray<FLootRolledModifier> Rolled = RollModifiers(Modifiers, Stream, LevelMultiplier, Level);

	FInventoryItemManifest Manifest;
	Manifest.Fragments = Template.FixedFragments;
	AddFragment(Manifest, LootItemText::MakeInfo(Template, PowerGrid, Rolled, Level, Params.Rarity));
	AddFragment(Manifest, PowerGrid);
	if (!Rolled.IsEmpty())
	{
		AddFragment(Manifest, MakeEffects(Rolled));
	}
	AddFragment(Manifest, MakeOrigin(Template, Params, Modifiers));
	return Manifest;
}
