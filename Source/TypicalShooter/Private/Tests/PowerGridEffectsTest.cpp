// Threshold effects of items (power grid R13-R14a, R13a): every reached step (power fed to the item >= threshold) is
// active; character effects act on the whole character, attack bonuses only on the slots of connected active receivers.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "PowerGrid/PowerEffectsComponent.h"
#include "PowerGrid/PowerEffectsFragment.h"
#include "Tests/PowerGridTestEffect.h"
#include "Tests/PowerGridTestFixture.h"
#include "TypicalShooterCharacter.h"

namespace PowerGridEffectsTest
{
	/** One step at Threshold: the test character effect, or an attack bonus when Bonus is set. */
	FPowerThresholdEffect Step(const int32 Threshold, const TOptional<FAttackStatBonus> Bonus = {})
	{
		FPowerThresholdEffect Result;
		Result.Threshold = Threshold;
		if (Bonus)
		{
			Result.AttackBonuses.Add(*Bonus);
		}
		else
		{
			Result.CharacterEffect = UPowerGridTestEffect::StaticClass();
		}
		return Result;
	}

	/** Adds the effects fragment to Manifest (a manifest without a power grid fragment is outside the grid). */
	FInventoryItemManifest WithEffects(FInventoryItemManifest Manifest, const TArray<FPowerThresholdEffect>& Steps)
	{
		FPowerEffects_ItemFragment Fragment;
		Fragment.Effects = Steps;
		Manifest.Fragments.Add(TInstancedStruct<FItemFragment>::Make<FPowerEffects_ItemFragment>(Fragment));
		return Manifest;
	}

	FInventoryItemManifest Source(const int32 Power)
	{
		FInventoryItemManifest Result = PowerGridTest::Manifest(EPowerItemType::Source);
		Result.Fragments[0].GetMutable<FPowerGrid_ItemFragment>().Source.Power = Power;
		return Result;
	}

	struct FEffectsFixture : PowerGridTest::FArmed
	{
		explicit FEffectsFixture(const ShootingTest::FWorldFixture& F) : FArmed(F)
		{
			NewObject<UPowerEffectsComponent>(Character)->RegisterComponent();
		}

		int32 TestEffects() const
		{
			return Asc->GetGameplayEffectCount(UPowerGridTestEffect::StaticClass(), nullptr);
		}

		float Damage(const EAttackSlot Slot) const
		{
			return Asc->GetNumericAttribute(UAttackAttributeSet::GetAttribute(Slot, EAttackAttribute::DamagePhysical));
		}
	};

	const FAttackStatBonus PlusFiveDamage = { EAttackAttribute::DamagePhysical, 5.f };
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerGridEffectThresholdsTest, "TypicalShooter.PowerGrid.Effects.CharacterThresholds", ShootingTest::Flags)
bool FPowerGridEffectThresholdsTest::RunTest(const FString& Parameters)
{
	using namespace PowerGridEffectsTest;
	const ShootingTest::FWorldFixture F(true);
	const FEffectsFixture A(F);

	// Steps 0, 1 and 3 on a connector next to a power 1 source: 0 and 1 reached, they stack (R13).
	UInventoryItem* Core = A.Inventory->AddItem(Source(1), FHhAxialCoord(0, 0));
	UInventoryItem* Booster = A.Inventory->AddItem(
		WithEffects(PowerGridTest::Manifest(EPowerItemType::Connector), { Step(0), Step(1), Step(3) }), FHhAxialCoord(0, 1));
	TestEqual(TEXT("Steps 0 and 1 active at power 1"), A.TestEffects(), 2);

	A.Inventory->AddItem(Source(2), FHhAxialCoord(1, 1));
	TestEqual(TEXT("Step 3 joins at power 3 (R13a)"), A.TestEffects(), 3);

	A.Inventory->RemoveItem(Core);
	TestEqual(TEXT("Back to power 2: step 3 off"), A.TestEffects(), 2);

	A.Inventory->MoveItem(Booster, FHhAxialCoord(-2, 0));
	TestEqual(TEXT("Unconnected: only step 0 stays (R13)"), A.TestEffects(), 1);

	A.Inventory->AddItem(WithEffects(FInventoryItemManifest(), { Step(0), Step(1) }), FHhAxialCoord(2, -1));
	TestEqual(TEXT("An item outside the grid has power 0: its step 0 works"), A.TestEffects(), 2);

	A.Inventory->RemoveItem(Booster);
	TestEqual(TEXT("A removed item's effects go away"), A.TestEffects(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerGridEffectLevelTest, "TypicalShooter.PowerGrid.Effects.CharacterEffectLevel", ShootingTest::Flags)
bool FPowerGridEffectLevelTest::RunTest(const FString& Parameters)
{
	using namespace PowerGridEffectsTest;
	const ShootingTest::FWorldFixture F(true);
	const FEffectsFixture A(F);

	FPowerThresholdEffect Leveled = Step(0);
	Leveled.CharacterEffectLevel = 7.f;
	A.Inventory->AddItem(WithEffects(FInventoryItemManifest(), { Leveled }), FHhAxialCoord(0, 0));

	FGameplayEffectQuery Query;
	Query.EffectDefinition = UPowerGridTestEffect::StaticClass();
	const TArray<FActiveGameplayEffectHandle> Handles = A.Asc->GetActiveEffects(Query);
	if (TestEqual(TEXT("The step's effect is active"), Handles.Num(), 1))
	{
		const FActiveGameplayEffect* Active = A.Asc->GetActiveGameplayEffect(Handles[0]);
		TestEqual(TEXT("Effect is applied with the step's level"), Active ? Active->Spec.GetLevel() : 0.f, 7.f);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerGridEffectAttackBonusTest, "TypicalShooter.PowerGrid.Effects.AttackBonusesToConnectedReceivers", ShootingTest::Flags)
bool FPowerGridEffectAttackBonusTest::RunTest(const FString& Parameters)
{
	using namespace PowerGridEffectsTest;
	const ShootingTest::FWorldFixture F(true);
	const FEffectsFixture A(F);
	const TArray<FAttackStatBonus> Base = { { EAttackAttribute::DamagePhysical, 10.f } };

	A.Inventory->AddItem(Source(1), FHhAxialCoord(0, 0));
	A.Inventory->AddItem(PowerGridTest::Manifest(EPowerItemType::Receiver, Base), ReceiverSlots::GetLocation(EReceiverSlot::Primary));
	A.Inventory->AddItem(PowerGridTest::Manifest(EPowerItemType::Receiver, Base), ReceiverSlots::GetLocation(EReceiverSlot::Alternate));

	// (1,-1) touches the source and the primary receiver only.
	UInventoryItem* Booster = A.Inventory->AddItem(
		WithEffects(PowerGridTest::Manifest(EPowerItemType::Connector), { Step(1, PlusFiveDamage), Step(2, PlusFiveDamage) }), FHhAxialCoord(1, -1));
	TestEqual(TEXT("Connected primary receiver gets the reached bonus (R14, R14a)"), A.Damage(EAttackSlot::Primary), 15.f);
	TestEqual(TEXT("The bonus stacks on the receiver base"), A.BaseOf(EAttackSlot::Primary, EAttackAttribute::DamagePhysical), 10.f);
	TestEqual(TEXT("Unconnected alternate receiver is untouched"), A.Damage(EAttackSlot::Alternate), 10.f);

	A.Inventory->AddItem(WithEffects(FInventoryItemManifest(), { Step(0, PlusFiveDamage) }), FHhAxialCoord(2, -2));
	TestEqual(TEXT("A bonus of an unconnected item does nothing"), A.Damage(EAttackSlot::Primary), 15.f);

	A.Inventory->RemoveItem(A.Inventory->GetItemAt(FHhAxialCoord(0, 0)));
	A.Inventory->AddItem(WithEffects(Source(2), { Step(1, PlusFiveDamage) }), FHhAxialCoord(0, 0));
	TestEqual(TEXT("Both steps of the booster at power 2, plus the source's own bonus"), A.Damage(EAttackSlot::Primary), 25.f);
	TestEqual(TEXT("The source feeds both receivers: its bonus reaches the alternate too"), A.Damage(EAttackSlot::Alternate), 15.f);

	A.Inventory->MoveItem(Booster, FHhAxialCoord(-2, 0));
	TestEqual(TEXT("Moving the booster away removes its bonuses (R13a)"), A.Damage(EAttackSlot::Primary), 15.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerGridEffectsOnCharacterTest, "TypicalShooter.PowerGrid.Effects.OnCharacter", ShootingTest::Flags)
bool FPowerGridEffectsOnCharacterTest::RunTest(const FString& Parameters)
{
	TestNotNull(TEXT("Character applies item effects"), GetDefault<ATypicalShooterCharacter>()->FindComponentByClass<UPowerEffectsComponent>());
	return true;
}

#endif
