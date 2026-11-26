// Tests for the generated name, description and icon (LootGeneration R7).

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/Texture2D.h"
#include "LootTestFixture.h"
#include "Loot/LootGenerator.h"
#include "Loot/LootTags.h"

namespace LootInfoTest
{
	using namespace LootTestFixture;

	ULootItemTemplate* SourceTemplate()
	{
		ULootItemTemplate* Template = MakeTemplate(EPowerItemType::Source, {}, false);
		Template->DisplayName = FText::FromString(TEXT("Particle Core"));
		Template->Source.Power = FLootIntRange(4, 4);
		Template->Source.Capacity = FLootIntRange(12, 12);
		Template->Source.RechargeDelay = FLootFloatRange(0.5f, 0.5f);
		return Template;
	}

	FString DescriptionOf(const FInventoryItemManifest& Manifest)
	{
		const FInfo_ItemFragment* Info = Manifest.GetFragmentOfType<FInfo_ItemFragment>();
		return Info ? Info->Description.ToString() : TEXT("<no info fragment>");
	}

	ULootModifier* TextModifier(const int32 Tier, const int32 Threshold, const float Bonus, const TCHAR* Line)
	{
		ULootModifier* Modifier = MakeModifier(Tier, LootTags::Loot_Modifier_Attack, Threshold);
		FLootAttackStatRange& Range = Modifier->AttackBonuses.AddDefaulted_GetRef();
		Range.Value = FLootFloatRange(Bonus, Bonus);
		Modifier->DescriptionLine = FText::FromString(Line);
		return Modifier;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootInfoDescriptionTest, "TypicalShooter.Loot.Generator.Info.Description", LootTestFixture::Flags)
bool FLootInfoDescriptionTest::RunTest(const FString& Parameters)
{
	using namespace LootInfoTest;
	ULootItemTemplate* Template = SourceTemplate();
	Template->DescriptionFormat = FText::FromString(TEXT("Power {Power}, clip {Capacity}, recharge {RechargeDelay}s, level {Level} {Rarity}"));
	ULootRarity* Rarity = MakeRarity({});
	Rarity->DisplayName = FText::FromString(TEXT("Rare"));

	const FInventoryItemManifest Manifest = LootGenerator::Generate(*Template, {10, 1, Rarity});
	const FInfo_ItemFragment* Info = Manifest.GetFragmentOfType<FInfo_ItemFragment>();
	if (!TestNotNull(TEXT("Generated item has an info fragment"), Info))
	{
		return false;
	}
	TestEqual(TEXT("Name comes from the template"), Info->DisplayName.ToString(), FString(TEXT("Particle Core")));
	TestEqual(TEXT("Placeholders are filled with rolled values"), Info->Description.ToString(),
		FString(TEXT("Power 4, clip 12, recharge 0.5s, level 10 Rare")));

	const FInventoryItemManifest NoRarity = LootGenerator::Generate(*Template, {10, 1});
	TestEqual(TEXT("No rarity = empty {Rarity}"), DescriptionOf(NoRarity), FString(TEXT("Power 4, clip 12, recharge 0.5s, level 10 ")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootInfoModifierLinesTest, "TypicalShooter.Loot.Generator.Info.ModifierLines", LootTestFixture::Flags)
bool FLootInfoModifierLinesTest::RunTest(const FString& Parameters)
{
	using namespace LootInfoTest;
	ULootItemTemplate* Template = SourceTemplate();
	Template->DescriptionFormat = FText::FromString(TEXT("Power {Power}"));
	AddToPool(*Template, TextModifier(2, 5, 7.f, TEXT("+{Bonus0} damage from {Threshold} power")));
	AddToPool(*Template, TextModifier(1, 1, 2.5f, TEXT("+{Bonus0} acid from {Threshold}")));
	const ULootRarity* Rarity = MakeRarity({{2, LootTags::Loot_Modifier_Attack}, {1, LootTags::Loot_Modifier_Attack}});

	TestEqual(TEXT("Modifier lines follow the description in threshold order"),
		DescriptionOf(LootGenerator::Generate(*Template, {1, 1, Rarity})),
		FString(TEXT("Power 4\n+2.5 acid from 1\n+7 damage from 5 power")));

	Template->DescriptionFormat = FText::GetEmpty();
	TestEqual(TEXT("Without a description only the lines remain"),
		DescriptionOf(LootGenerator::Generate(*Template, {1, 1, Rarity})),
		FString(TEXT("+2.5 acid from 1\n+7 damage from 5 power")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootInfoIconTest, "TypicalShooter.Loot.Generator.Info.Icon", LootTestFixture::Flags)
bool FLootInfoIconTest::RunTest(const FString& Parameters)
{
	using namespace LootInfoTest;
	ULootItemTemplate* Template = SourceTemplate();
	const ULootRarity* Rare = MakeRarity({});
	const ULootRarity* Unlisted = MakeRarity({});
	UTexture2D* RareIcon = NewObject<UTexture2D>();
	UTexture2D* DefaultIcon = NewObject<UTexture2D>();
	Template->IconsByRarity.Add(Rare, RareIcon);
	Template->DefaultIcon = DefaultIcon;

	auto IconOf = [&](const ULootRarity* Rarity) -> const UTexture2D*
	{
		const FInventoryItemManifest Manifest = LootGenerator::Generate(*Template, {1, 1, Rarity});
		const FInfo_ItemFragment* Info = Manifest.GetFragmentOfType<FInfo_ItemFragment>();
		return Info ? Info->Icon.Get() : nullptr;
	};
	TestEqual(TEXT("Icon of the rarity"), IconOf(Rare), static_cast<const UTexture2D*>(RareIcon));
	TestEqual(TEXT("Unlisted rarity falls back to the default icon"), IconOf(Unlisted), static_cast<const UTexture2D*>(DefaultIcon));
	TestEqual(TEXT("No rarity falls back to the default icon"), IconOf(nullptr), static_cast<const UTexture2D*>(DefaultIcon));
	return true;
}

#endif
