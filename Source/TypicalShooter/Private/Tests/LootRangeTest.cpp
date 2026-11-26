// Tests for loot value ranges and the level multiplier (LootGeneration R2).

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Loot/LootRange.h"
#include "Loot/LootSettings.h"

namespace LootRangeTest
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
	constexpr int32 Rolls = 200;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootRangeIntBoundsTest, "TypicalShooter.Loot.Range.IntWithinBounds", LootRangeTest::Flags)
bool FLootRangeIntBoundsTest::RunTest(const FString& Parameters)
{
	FRandomStream Stream(42);
	const FLootIntRange Range(2, 5);
	TSet<int32> Seen;
	for (int32 Index = 0; Index < LootRangeTest::Rolls; ++Index)
	{
		const int32 Value = Range.Roll(Stream, 3.f);
		if (!TestTrue(FString::Printf(TEXT("Non-scaling value %d stays in [2, 5]"), Value), Value >= 2 && Value <= 5))
		{
			return false;
		}
		Seen.Add(Value);
	}
	TestEqual(TEXT("Every value of the range comes up"), Seen.Num(), 4);
	TestEqual(TEXT("Single-value range"), FLootIntRange(7, 7).Roll(Stream, 1.f), 7);
	const int32 Swapped = FLootIntRange(5, 2).Roll(Stream, 1.f);
	TestTrue(TEXT("Min > Max is order-agnostic"), Swapped >= 2 && Swapped <= 5);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootRangeIntScaleTest, "TypicalShooter.Loot.Range.IntScalesWithLevel", LootRangeTest::Flags)
bool FLootRangeIntScaleTest::RunTest(const FString& Parameters)
{
	FRandomStream Stream(7);
	TestEqual(TEXT("Scaling range is multiplied and rounded"), FLootIntRange(3, 3, true).Roll(Stream, 2.5f), 8);
	TestEqual(TEXT("Non-scaling range ignores the multiplier"), FLootIntRange(3, 3, false).Roll(Stream, 2.5f), 3);
	TestEqual(TEXT("Result is never negative"), FLootIntRange(-4, -4, true).Roll(Stream, 2.f), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootRangeFloatTest, "TypicalShooter.Loot.Range.Float", LootRangeTest::Flags)
bool FLootRangeFloatTest::RunTest(const FString& Parameters)
{
	FRandomStream Stream(3);
	const FLootFloatRange Range(0.5f, 1.5f);
	bool bAnyDifferent = false;
	const float First = Range.Roll(Stream, 1.f);
	for (int32 Index = 0; Index < LootRangeTest::Rolls; ++Index)
	{
		const float Value = Range.Roll(Stream, 4.f);
		if (!TestTrue(TEXT("Non-scaling value stays in [0.5, 1.5]"), Value >= 0.5f && Value <= 1.5f))
		{
			return false;
		}
		bAnyDifferent |= !FMath::IsNearlyEqual(Value, First);
	}
	TestTrue(TEXT("Values vary"), bAnyDifferent);
	TestEqual(TEXT("Scaling range is multiplied"), FLootFloatRange(1.5f, 1.5f, true).Roll(Stream, 2.f), 3.f);
	TestEqual(TEXT("Result is never negative"), FLootFloatRange(-1.f, -1.f).Roll(Stream, 1.f), 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootRangeDeterminismTest, "TypicalShooter.Loot.Range.Deterministic", LootRangeTest::Flags)
bool FLootRangeDeterminismTest::RunTest(const FString& Parameters)
{
	FRandomStream StreamA(1234);
	FRandomStream StreamB(1234);
	const FLootIntRange IntRange(0, 1000);
	const FLootFloatRange FloatRange(0.f, 1000.f);
	for (int32 Index = 0; Index < 20; ++Index)
	{
		TestEqual(TEXT("Same seed -> same int"), IntRange.Roll(StreamA, 1.f), IntRange.Roll(StreamB, 1.f));
		TestEqual(TEXT("Same seed -> same float"), FloatRange.Roll(StreamA, 1.f), FloatRange.Roll(StreamB, 1.f));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootSettingsLevelMultiplierTest, "TypicalShooter.Loot.Range.LevelMultiplier", LootRangeTest::Flags)
bool FLootSettingsLevelMultiplierTest::RunTest(const FString& Parameters)
{
	const ULootSettings* Settings = GetDefault<ULootSettings>();
	TestEqual(TEXT("Level is clamped from below"), ULootSettings::ClampLevel(0), 1);
	TestEqual(TEXT("Level is clamped from above"), ULootSettings::ClampLevel(150), 100);
	TestEqual(TEXT("Level inside the range is kept"), ULootSettings::ClampLevel(42), 42);
	TestEqual(TEXT("Default multiplier at level 1"), Settings->GetLevelMultiplier(1), 1.f, 0.001f);
	TestEqual(TEXT("Default multiplier at level 100"), Settings->GetLevelMultiplier(100), 5.95f, 0.001f);
	TestEqual(TEXT("Default multiplier grows 5% per level"), Settings->GetLevelMultiplier(21), 2.f, 0.001f);
	TestEqual(TEXT("Multiplier uses the clamped level"), Settings->GetLevelMultiplier(500), 5.95f, 0.001f);

	ULootSettings* Empty = NewObject<ULootSettings>();
	Empty->LevelMultiplier.GetRichCurve()->Reset();
	TestEqual(TEXT("Empty curve means no scaling"), Empty->GetLevelMultiplier(50), 1.f);
	return true;
}

#endif
