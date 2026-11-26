// Tests for rarity slots picking modifiers and modifiers rolling threshold steps (LootGeneration R3, R4, R8).

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "GameplayEffect.h"
#include "LootTestFixture.h"
#include "Loot/LootGenerator.h"
#include "Loot/LootModifierPicker.h"
#include "Loot/LootTags.h"

namespace LootModifiersTest
{
	using namespace LootTestFixture;

	/** Picked modifiers for many seeds, as a set. */
	TSet<const ULootModifier*> PickedOverSeeds(const ULootItemTemplate& Template, const ULootRarity* Rarity, const int32 Seeds = 60)
	{
		TSet<const ULootModifier*> Seen;
		for (int32 Seed = 0; Seed < Seeds; ++Seed)
		{
			FRandomStream Stream(Seed);
			Seen.Append(LootModifierPicker::Pick(Template.ModifierPool, Rarity, Stream));
		}
		return Seen;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootModifiersSlotMatchTest, "TypicalShooter.Loot.Modifiers.SlotMatchesTagAndTier", LootTestFixture::Flags)
bool FLootModifiersSlotMatchTest::RunTest(const FString& Parameters)
{
	using namespace LootModifiersTest;
	ULootItemTemplate* Template = MakeTemplate(EPowerItemType::Connector, {}, false);
	const ULootModifier* AttackT1 = MakeModifier(1, LootTags::Loot_Modifier_Attack);
	const ULootModifier* AttackT2 = MakeModifier(2, LootTags::Loot_Modifier_Attack);
	const ULootModifier* CharacterT1 = MakeModifier(1, LootTags::Loot_Modifier_Character);
	AddToPool(*Template, AttackT1);
	AddToPool(*Template, AttackT2);
	AddToPool(*Template, CharacterT1);

	TestEqual(TEXT("Attack tier 1 slot takes only the attack tier 1 modifier"),
		PickedOverSeeds(*Template, MakeRarity({{1, LootTags::Loot_Modifier_Attack}})).Array(), TArray<const ULootModifier*>{AttackT1});
	TestEqual(TEXT("Tier must be equal, not lower"),
		PickedOverSeeds(*Template, MakeRarity({{2, LootTags::Loot_Modifier_Attack}})).Array(), TArray<const ULootModifier*>{AttackT2});

	const TSet<const ULootModifier*> ParentTag = PickedOverSeeds(*Template, MakeRarity({{1, LootTags::Loot_Modifier}}));
	TestTrue(TEXT("Parent tag slot takes child-tagged modifiers"), ParentTag.Contains(AttackT1) && ParentTag.Contains(CharacterT1));
	TestFalse(TEXT("Parent tag slot still respects the tier"), ParentTag.Contains(AttackT2));

	const TSet<const ULootModifier*> AnyTag = PickedOverSeeds(*Template, MakeRarity({{1, FGameplayTag()}}));
	TestTrue(TEXT("Empty slot tag takes any tag of the tier"), AnyTag.Contains(AttackT1) && AnyTag.Contains(CharacterT1));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootModifiersNoRepeatTest, "TypicalShooter.Loot.Modifiers.NoRepeatsAndSkip", LootTestFixture::Flags)
bool FLootModifiersNoRepeatTest::RunTest(const FString& Parameters)
{
	using namespace LootModifiersTest;
	ULootItemTemplate* Template = MakeTemplate(EPowerItemType::Connector, {}, false);
	const ULootModifier* First = MakeModifier(1, LootTags::Loot_Modifier_Attack);
	const ULootModifier* Second = MakeModifier(1, LootTags::Loot_Modifier_Attack);
	const ULootModifier* NeverPicked = MakeModifier(1, LootTags::Loot_Modifier_Attack);
	AddToPool(*Template, First);
	AddToPool(*Template, Second);
	AddToPool(*Template, NeverPicked, 0.f);
	AddExpectedMessage(TEXT("has no modifier for slot"), EAutomationExpectedMessageFlags::Contains, 0);
	const ULootRarity* ThreeSlots =MakeRarity({{1, LootTags::Loot_Modifier_Attack}, {1, LootTags::Loot_Modifier_Attack}, {1, LootTags::Loot_Modifier_Attack}});

	for (int32 Seed = 0; Seed < 30; ++Seed)
	{
		FRandomStream Stream(Seed);
		const TArray<const ULootModifier*> Picked = LootModifierPicker::Pick(Template->ModifierPool, ThreeSlots, Stream);
		TestEqual(TEXT("The slot without a candidate is skipped"), Picked.Num(), 2);
		TestTrue(TEXT("No modifier twice"), Picked.Contains(First) && Picked.Contains(Second));
	}

	FRandomStream Stream(1);
	TestEqual(TEXT("No rarity = no modifiers"), LootModifierPicker::Pick(Template->ModifierPool, nullptr, Stream).Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootModifiersRollTest, "TypicalShooter.Loot.Modifiers.RollStep", LootTestFixture::Flags)
bool FLootModifiersRollTest::RunTest(const FString& Parameters)
{
	using namespace LootTestFixture;
	ULootModifier* Modifier = MakeModifier(1, LootTags::Loot_Modifier_Attack);
	Modifier->Threshold = FLootIntRange(2, 4);
	FLootAttackStatRange& Bonus = Modifier->AttackBonuses.AddDefaulted_GetRef();
	Bonus.Attribute = EAttackAttribute::DamageAcid;
	Bonus.Value = FLootFloatRange(5.f, 5.f, true);
	Modifier->CharacterEffect = UGameplayEffect::StaticClass();

	FRandomStream Stream(4);
	const FPowerThresholdEffect Step = Modifier->Roll(Stream, 3.f);
	TestTrue(FString::Printf(TEXT("Threshold %d in [2, 4]"), Step.Threshold), Step.Threshold >= 2 && Step.Threshold <= 4);
	TestEqual(TEXT("Character effect is copied"), Step.CharacterEffect.Get(), UGameplayEffect::StaticClass());
	if (TestEqual(TEXT("Every bonus is rolled"), Step.AttackBonuses.Num(), 1))
	{
		TestEqual(TEXT("Bonus attribute"), Step.AttackBonuses[0].Attribute, EAttackAttribute::DamageAcid);
		TestEqual(TEXT("Bonus scales with level"), Step.AttackBonuses[0].Value, 15.f);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootModifiersGeneratorTest, "TypicalShooter.Loot.Generator.Modifiers", LootTestFixture::Flags)
bool FLootModifiersGeneratorTest::RunTest(const FString& Parameters)
{
	using namespace LootTestFixture;
	ULootItemTemplate* Template = MakeTemplate(EPowerItemType::Connector, {}, false);
	const ULootModifier* High = MakeModifier(2, LootTags::Loot_Modifier_Attack, 5);
	const ULootModifier* Low = MakeModifier(1, LootTags::Loot_Modifier_Character, 1);
	AddToPool(*Template, High);
	AddToPool(*Template, Low);
	const ULootRarity* Rarity = MakeRarity({{2, LootTags::Loot_Modifier_Attack}, {1, LootTags::Loot_Modifier_Character}});

	const FInventoryItemManifest Manifest = LootGenerator::Generate(*Template, {1, 3, Rarity});
	const FPowerEffects_ItemFragment* Effects = Manifest.GetFragmentOfType<FPowerEffects_ItemFragment>();
	if (TestNotNull(TEXT("Item with modifiers gets a power effects fragment"), Effects)
		&& TestEqual(TEXT("One step per modifier"), Effects->Effects.Num(), 2))
	{
		TestEqual(TEXT("Steps are sorted by threshold"), Effects->Effects[0].Threshold, 1);
		TestEqual(TEXT("Higher threshold last"), Effects->Effects[1].Threshold, 5);
	}

	const FLootOrigin_ItemFragment* Origin = OriginOf(Manifest);
	if (TestNotNull(TEXT("Origin fragment"), Origin))
	{
		TestEqual(TEXT("Origin keeps the rarity"), Origin->Rarity.Get(), Rarity);
		TestEqual(TEXT("Origin keeps the modifiers in slot order"), Origin->Modifiers, TArray<TObjectPtr<const ULootModifier>>{High, Low});
	}

	const FInventoryItemManifest Plain = LootGenerator::Generate(*Template, {1, 3});
	TestNull(TEXT("No rarity = no power effects fragment"), Plain.GetFragmentOfType<FPowerEffects_ItemFragment>());
	return true;
}

#endif
