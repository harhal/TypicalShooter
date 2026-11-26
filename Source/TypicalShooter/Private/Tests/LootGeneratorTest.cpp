// Tests for LootGenerator: shape, fixed fragments, determinism, origin (LootGeneration R1, R6, R8).

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "LootTestFixture.h"
#include "Loot/LootGenerator.h"
#include "Loot/LootRandom.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootRandomPickWeightedTest, "TypicalShooter.Loot.Random.PickWeighted", LootTestFixture::Flags)
bool FLootRandomPickWeightedTest::RunTest(const FString& Parameters)
{
	FRandomStream Stream(5);
	TestEqual(TEXT("Empty weights pick nothing"), LootRandom::PickWeightedIndex(Stream, {}), INDEX_NONE);
	TestEqual(TEXT("Zero / negative weights pick nothing"), LootRandom::PickWeightedIndex(Stream, {0.f, -2.f}), INDEX_NONE);

	int32 Counts[3] = {};
	for (int32 Index = 0; Index < 4000; ++Index)
	{
		const int32 Picked = LootRandom::PickWeightedIndex(Stream, {3.f, 0.f, 1.f});
		if (!TestTrue(TEXT("Picked index is valid"), Picked >= 0 && Picked < 3))
		{
			return false;
		}
		++Counts[Picked];
	}
	TestEqual(TEXT("Zero weight never wins"), Counts[1], 0);
	TestTrue(FString::Printf(TEXT("Weights 3:1 give ~75%% (%d of 4000)"), Counts[0]), Counts[0] > 2800 && Counts[0] < 3200);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootGeneratorShapeTest, "TypicalShooter.Loot.Generator.Shape", LootTestFixture::Flags)
bool FLootGeneratorShapeTest::RunTest(const FString& Parameters)
{
	using namespace LootTestFixture;
	const int32 Bent = MaskOf({EHexFace::Right, EHexFace::BottomLeft});
	const int32 Straight = MaskOf({EHexFace::Right, EHexFace::Left});
	const ULootItemTemplate* Fixed = MakeTemplate(EPowerItemType::Source, {Shape(Bent), Shape(Straight, 0.f)}, false);

	for (int32 Seed = 0; Seed < 30; ++Seed)
	{
		const FInventoryItemManifest Manifest = LootGenerator::Generate(*Fixed, {1, Seed});
		const FPowerGrid_ItemFragment* PowerGrid = PowerGridOf(Manifest);
		if (!TestNotNull(TEXT("Generated item has a power grid fragment"), PowerGrid))
		{
			return false;
		}
		TestEqual(TEXT("Type comes from the template"), PowerGrid->ItemType, EPowerItemType::Source);
		TestEqual(TEXT("Only the shape with weight wins, not rotated"), PowerGrid->OpenFaces, Bent);
	}

	const ULootItemTemplate* Rotating = MakeTemplate(EPowerItemType::Connector, {Shape(Bent)}, true);
	TSet<int32> Masks;
	for (int32 Seed = 0; Seed < 60; ++Seed)
	{
		const int32 Mask = OpenFacesOf(LootGenerator::Generate(*Rotating, {1, Seed}));
		bool bIsRotation = false;
		for (int32 Steps = 0; Steps < HexFaces::Num; ++Steps)
		{
			bIsRotation |= HexFaces::RotateMask(Bent, Steps) == Mask;
		}
		TestTrue(TEXT("Rotated mask is a rotation of the shape"), bIsRotation);
		Masks.Add(Mask);
	}
	TestEqual(TEXT("All six rotations come up"), Masks.Num(), HexFaces::Num);

	const ULootItemTemplate* NoShapes = MakeTemplate(EPowerItemType::Connector, {}, true);
	TestEqual(TEXT("No shapes = all faces open"), OpenFacesOf(LootGenerator::Generate(*NoShapes, {1, 3})), HexFaces::AllFaces);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootGeneratorFixedFragmentsTest, "TypicalShooter.Loot.Generator.FixedFragments", LootTestFixture::Flags)
bool FLootGeneratorFixedFragmentsTest::RunTest(const FString& Parameters)
{
	using namespace LootTestFixture;
	ULootItemTemplate* Template = MakeTemplate(EPowerItemType::Connector, {}, false);
	FGrid_ItemFragment Grid;
	Grid.Size = 3;
	Template->FixedFragments.Add(TInstancedStruct<FItemFragment>::Make<FGrid_ItemFragment>(Grid));

	const FInventoryItemManifest Manifest = LootGenerator::Generate(*Template, {1, 1});
	const FGrid_ItemFragment* Copied = Manifest.GetFragmentOfType<FGrid_ItemFragment>();
	if (TestNotNull(TEXT("Fixed fragment is copied"), Copied))
	{
		TestEqual(TEXT("Fixed fragment keeps its values"), Copied->Size, 3);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootGeneratorDeterminismTest, "TypicalShooter.Loot.Generator.Determinism", LootTestFixture::Flags)
bool FLootGeneratorDeterminismTest::RunTest(const FString& Parameters)
{
	using namespace LootTestFixture;
	const ULootItemTemplate* Template = MakeTemplate(EPowerItemType::Connector,
		{Shape(MaskOf({EHexFace::Right})), Shape(MaskOf({EHexFace::Right, EHexFace::Left})), Shape(MaskOf({EHexFace::Right, EHexFace::TopLeft}))}, true);

	TSet<int32> Masks;
	for (int32 Seed = 0; Seed < 20; ++Seed)
	{
		const int32 First = OpenFacesOf(LootGenerator::Generate(*Template, {5, Seed}));
		const int32 Second = OpenFacesOf(LootGenerator::Generate(*Template, {5, Seed}));
		TestEqual(TEXT("Same seed -> same item"), First, Second);
		Masks.Add(First);
	}
	TestTrue(TEXT("Different seeds give different items"), Masks.Num() > 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootGeneratorOriginTest, "TypicalShooter.Loot.Generator.Origin", LootTestFixture::Flags)
bool FLootGeneratorOriginTest::RunTest(const FString& Parameters)
{
	using namespace LootTestFixture;
	const ULootItemTemplate* Template = MakeTemplate(EPowerItemType::Connector, {}, false);

	const FInventoryItemManifest Manifest = LootGenerator::Generate(*Template, {250, 77});
	const FLootOrigin_ItemFragment* Origin = OriginOf(Manifest);
	if (!TestNotNull(TEXT("Generated item has an origin fragment"), Origin))
	{
		return false;
	}
	TestEqual(TEXT("Origin keeps the template"), Origin->Template.Get(), Template);
	TestEqual(TEXT("Origin keeps the clamped level"), Origin->Level, 100);
	TestEqual(TEXT("Origin keeps the seed"), Origin->Seed, 77);
	return true;
}

#endif
