// Receiver base stats: while a receiver works in a slot its base parameters are the base values of that slot's attack
// attributes; otherwise the slot falls back to the neutral defaults (power grid R5a).

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Shooting/ShootingTags.h"
#include "Tests/PowerGridTestFixture.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FReceiverBaseStatsTest, "TypicalShooter.PowerGrid.Receivers.BaseStatsToSlot", ShootingTest::Flags)
bool FReceiverBaseStatsTest::RunTest(const FString& Parameters)
{
	using namespace PowerGridTest;
	const ShootingTest::FWorldFixture F(true);
	const FArmed A(F);
	const TArray<FAttackStatBonus> Rifle = {
		{ EAttackAttribute::DamagePhysical, 12.f }, { EAttackAttribute::ShotsPerBurst, 3.f }, { EAttackAttribute::FireEndDistance, 4000.f } };
	const float NeutralRange = UAttackAttributeSet::GetDefaultValue(EAttackAttribute::FireEndDistance);

	UInventoryItem* Source = A.Inventory->AddItem(Manifest(EPowerItemType::Source), FHhAxialCoord(0, 0));
	UInventoryItem* Receiver = A.Inventory->AddItem(Manifest(EPowerItemType::Receiver, Rifle), ReceiverSlots::GetLocation(EReceiverSlot::Primary));
	TestEqual(TEXT("Receiver damage is the primary base (R5a)"), A.BaseOf(EAttackSlot::Primary, EAttackAttribute::DamagePhysical), 12.f);
	TestEqual(TEXT("Receiver burst is the primary base"), A.BaseOf(EAttackSlot::Primary, EAttackAttribute::ShotsPerBurst), 3.f);
	TestEqual(TEXT("Receiver range is the primary base"), A.BaseOf(EAttackSlot::Primary, EAttackAttribute::FireEndDistance), 4000.f);
	TestEqual(TEXT("Parameters the receiver does not set stay neutral"),
		A.BaseOf(EAttackSlot::Primary, EAttackAttribute::ProjectileSpeed), UAttackAttributeSet::GetDefaultValue(EAttackAttribute::ProjectileSpeed));
	TestEqual(TEXT("Alternate slot is untouched"), A.BaseOf(EAttackSlot::Alternate, EAttackAttribute::DamagePhysical), 0.f);
	TestEqual(TEXT("Shot stats read the receiver base"),
		UAttackAttributeSet::ReadStats(*A.Asc, EAttackSlot::Primary).Damage.Get(ShootingTags::Damage_Type_Physical), 12.f);

	A.Inventory->MoveItem(Receiver, ReceiverSlots::GetLocation(EReceiverSlot::Alternate));
	TestEqual(TEXT("Primary falls back to neutral damage"), A.BaseOf(EAttackSlot::Primary, EAttackAttribute::DamagePhysical), 0.f);
	TestEqual(TEXT("Primary falls back to a neutral burst"), A.BaseOf(EAttackSlot::Primary, EAttackAttribute::ShotsPerBurst), 1.f);
	TestEqual(TEXT("Primary falls back to the neutral range"), A.BaseOf(EAttackSlot::Primary, EAttackAttribute::FireEndDistance), NeutralRange);
	TestEqual(TEXT("The same base now drives the alternate slot"), A.BaseOf(EAttackSlot::Alternate, EAttackAttribute::DamagePhysical), 12.f);

	A.Inventory->RemoveItem(Source);
	TestEqual(TEXT("An unpowered receiver gives no base"), A.BaseOf(EAttackSlot::Alternate, EAttackAttribute::DamagePhysical), 0.f);
	return true;
}

#endif
