// Tests for loot tables: item count, template and rarity picks, determinism (LootGeneration R5, R6).

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "LootTestFixture.h"
#include "Loot/LootTable.h"

namespace LootTableTest
{
	using namespace LootTestFixture;

	void AddTemplate(ULootTable& Table, const ULootItemTemplate* Template, const float Weight = 1.f)
	{
		FLootTemplateEntry& Entry = Table.Templates.AddDefaulted_GetRef();
		Entry.Template = Template;
		Entry.Weight = Weight;
	}

	/** Rarity whose weight goes linearly from AtLevel1 to AtLevel100. */
	void AddRarity(ULootTable& Table, const ULootRarity* Rarity, const float AtLevel1, const float AtLevel100)
	{
		FLootRarityEntry& Entry = Table.Rarities.AddDefaulted_GetRef();
		Entry.Rarity = Rarity;
		FRichCurve* Curve = Entry.WeightByLevel.GetRichCurve();
		Curve->SetKeyInterpMode(Curve->AddKey(1.f, AtLevel1), RCIM_Linear);
		Curve->SetKeyInterpMode(Curve->AddKey(100.f, AtLevel100), RCIM_Linear);
	}

	TArray<const ULootRarity*> RaritiesOf(const TArray<FInventoryItemManifest>& Items)
	{
		TArray<const ULootRarity*> Result;
		for (const FInventoryItemManifest& Item : Items)
		{
			const FLootOrigin_ItemFragment* Origin = OriginOf(Item);
			Result.Add(Origin ? Origin->Rarity.Get() : nullptr);
		}
		return Result;
	}

	TArray<const ULootItemTemplate*> TemplatesOf(const TArray<FInventoryItemManifest>& Items)
	{
		TArray<const ULootItemTemplate*> Result;
		for (const FInventoryItemManifest& Item : Items)
		{
			const FLootOrigin_ItemFragment* Origin = OriginOf(Item);
			Result.Add(Origin ? Origin->Template.Get() : nullptr);
		}
		return Result;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootTableCountTest, "TypicalShooter.Loot.Table.CountAndDeterminism", LootTestFixture::Flags)
bool FLootTableCountTest::RunTest(const FString& Parameters)
{
	using namespace LootTableTest;
	ULootTable* Table = NewObject<ULootTable>();
	Table->ItemCount = FLootIntRange(2, 4);
	AddTemplate(*Table, MakeTemplate(EPowerItemType::Connector, {}, true));

	TSet<int32> Counts;
	for (int32 Seed = 0; Seed < 40; ++Seed)
	{
		const TArray<FInventoryItemManifest> Items = Table->Roll(5, Seed);
		TestTrue(FString::Printf(TEXT("Count %d in [2, 4]"), Items.Num()), Items.Num() >= 2 && Items.Num() <= 4);
		Counts.Add(Items.Num());
	}
	TestEqual(TEXT("Every count comes up"), Counts.Num(), 3);

	const TArray<FInventoryItemManifest> First = Table->Roll(5, 11);
	const TArray<FInventoryItemManifest> Second = Table->Roll(5, 11);
	if (TestEqual(TEXT("Same seed -> same count"), First.Num(), Second.Num()))
	{
		TSet<int32> ItemSeeds;
		for (int32 Index = 0; Index < First.Num(); ++Index)
		{
			TestEqual(TEXT("Same seed -> same item"), OriginOf(First[Index])->Seed, OriginOf(Second[Index])->Seed);
			TestEqual(TEXT("Items get the table level"), OriginOf(First[Index])->Level, 5);
			ItemSeeds.Add(OriginOf(First[Index])->Seed);
		}
		TestEqual(TEXT("Every item has its own seed"), ItemSeeds.Num(), First.Num());
	}

	ULootTable* Empty = NewObject<ULootTable>();
	TestEqual(TEXT("A table without templates gives nothing"), Empty->Roll(5, 1).Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootTableTemplatesTest, "TypicalShooter.Loot.Table.TemplateWeights", LootTestFixture::Flags)
bool FLootTableTemplatesTest::RunTest(const FString& Parameters)
{
	using namespace LootTableTest;
	ULootTable* Table = NewObject<ULootTable>();
	Table->ItemCount = FLootIntRange(10, 10);
	const ULootItemTemplate* Common = MakeTemplate(EPowerItemType::Connector, {}, false);
	const ULootItemTemplate* Never = MakeTemplate(EPowerItemType::Source, {}, false);
	const ULootItemTemplate* Other = MakeTemplate(EPowerItemType::Receiver, {}, false);
	AddTemplate(*Table, Common, 3.f);
	AddTemplate(*Table, Never, 0.f);
	AddTemplate(*Table, nullptr, 5.f);
	AddTemplate(*Table, Other, 1.f);

	int32 CommonCount = 0;
	int32 Total = 0;
	for (int32 Seed = 0; Seed < 40; ++Seed)
	{
		for (const ULootItemTemplate* Template : TemplatesOf(Table->Roll(1, Seed)))
		{
			TestTrue(TEXT("Zero weight and empty entries never win"), Template == Common || Template == Other);
			CommonCount += Template == Common;
			++Total;
		}
	}
	TestEqual(TEXT("Every item is generated"), Total, 400);
	TestTrue(FString::Printf(TEXT("Weights 3:1 give ~75%% (%d of %d)"), CommonCount, Total), CommonCount > 260 && CommonCount < 340);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootTableRaritiesTest, "TypicalShooter.Loot.Table.RarityWeightsByLevel", LootTestFixture::Flags)
bool FLootTableRaritiesTest::RunTest(const FString& Parameters)
{
	using namespace LootTableTest;
	ULootTable* Table = NewObject<ULootTable>();
	Table->ItemCount = FLootIntRange(20, 20);
	AddTemplate(*Table, MakeTemplate(EPowerItemType::Connector, {}, false));
	const ULootRarity* Common = MakeRarity({});
	const ULootRarity* Legendary = MakeRarity({});
	AddRarity(*Table, Common, 1.f, 0.f);
	AddRarity(*Table, Legendary, 0.f, 1.f);

	const TArray<const ULootRarity*> AtLevel1 = RaritiesOf(Table->Roll(1, 3));
	TestTrue(TEXT("Level 1: only common"), !AtLevel1.Contains(Legendary) && AtLevel1.Contains(Common));
	const TArray<const ULootRarity*> AtLevel100 = RaritiesOf(Table->Roll(100, 3));
	TestTrue(TEXT("Level 100: only legendary"), !AtLevel100.Contains(Common) && AtLevel100.Contains(Legendary));
	const TArray<const ULootRarity*> AtLevel50 = RaritiesOf(Table->Roll(50, 3));
	TestTrue(TEXT("Level 50: both"), AtLevel50.Contains(Common) && AtLevel50.Contains(Legendary));

	FLootRarityEntry Flat;
	TestEqual(TEXT("Empty weight curve = weight 1"), Flat.GetWeight(42), 1.f);

	ULootTable* NoRarities = NewObject<ULootTable>();
	NoRarities->ItemCount = FLootIntRange(1, 1);
	AddTemplate(*NoRarities, MakeTemplate(EPowerItemType::Connector, {}, false));
	TestEqual(TEXT("No rarities = item without rarity"), RaritiesOf(NoRarities->Roll(1, 1)), TArray<const ULootRarity*>{nullptr});
	return true;
}

#endif
