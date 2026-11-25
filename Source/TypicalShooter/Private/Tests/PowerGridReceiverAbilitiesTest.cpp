// Receiver abilities: the active receiver of a slot grants its ability with the slot's input; the air attack works
// only while airborne and fires down (power grid R5, R5b, R6a). Ticking world with a floor and an AI character.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "GameFramework/CharacterMovementComponent.h"
#include "Tests/PowerGridTestFixture.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FReceiverGrantsAbilityTest, "TypicalShooter.PowerGrid.Receivers.GrantsSlotAbility", ShootingTest::Flags)
bool FReceiverGrantsAbilityTest::RunTest(const FString& Parameters)
{
	using namespace PowerGridTest;
	const ShootingTest::FWorldFixture F(true);
	const FArmed A(F);
	TestEqual(TEXT("No receivers: no abilities (R6a)"), A.Asc->GetActivatableAbilities().Num(), 0);

	UInventoryItem* Source = A.Inventory->AddItem(Manifest(EPowerItemType::Source), FHhAxialCoord(0, 0));
	UInventoryItem* Receiver = A.Inventory->AddItem(Manifest(EPowerItemType::Receiver), ReceiverSlots::GetLocation(EReceiverSlot::Primary));
	const FGameplayAbilitySpec* Primary = A.FindSpec(EAttackInputID::PrimaryAttack);
	if (TestNotNull(TEXT("Powered receiver in the primary slot grants its ability on the primary button (R5)"), Primary))
	{
		TestTrue(TEXT("... as the primary attack"), A.SlotOf(Primary) == TOptional<EAttackSlot>(EAttackSlot::Primary));
		TestTrue(TEXT("... sourced from the receiver item"), Primary->SourceObject.Get() == Receiver);
	}
	TestEqual(TEXT("Exactly one ability"), A.Asc->GetActivatableAbilities().Num(), 1);

	A.Inventory->MoveItem(Receiver, ReceiverSlots::GetLocation(EReceiverSlot::Alternate));
	TestNull(TEXT("Leaving the slot takes the primary ability away"), A.FindSpec(EAttackInputID::PrimaryAttack));
	const FGameplayAbilitySpec* Alternate = A.FindSpec(EAttackInputID::AlternateAttack);
	if (TestNotNull(TEXT("The same receiver works in the alternate slot (R5b)"), Alternate))
	{
		TestTrue(TEXT("... as the alternate attack"), A.SlotOf(Alternate) == TOptional<EAttackSlot>(EAttackSlot::Alternate));
	}

	A.Inventory->MoveItem(Receiver, ReceiverSlots::GetLocation(EReceiverSlot::Air));
	const FGameplayAbilitySpec* Air = A.FindSpec(EAttackInputID::Jump);
	if (TestNotNull(TEXT("In the air slot it is bound to the jump button (R5)"), Air))
	{
		TestTrue(TEXT("... as the air attack"), A.SlotOf(Air) == TOptional<EAttackSlot>(EAttackSlot::Air));
	}

	A.Inventory->RemoveItem(Source);
	TestEqual(TEXT("Unpowered receiver grants nothing (R6)"), A.Asc->GetActivatableAbilities().Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FReceiverAirAttackTest, "TypicalShooter.PowerGrid.Receivers.AirAttackOnlyAirborne", ShootingTest::Flags)
bool FReceiverAirAttackTest::RunTest(const FString& Parameters)
{
	using namespace PowerGridTest;
	const ShootingTest::FWorldFixture F(true);
	FArmed A(F);
	A.Inventory->AddItem(Manifest(EPowerItemType::Source), FHhAxialCoord(0, 0));
	A.Inventory->AddItem(Manifest(EPowerItemType::Receiver), ReceiverSlots::GetLocation(EReceiverSlot::Air));
	constexpr int32 JumpInput = static_cast<int32>(EAttackInputID::Jump);

	F.Tick(1.f);
	TestTrue(TEXT("Standing on the floor"), A.Character->GetCharacterMovement()->IsMovingOnGround());
	A.Asc->AbilityLocalInputPressed(JumpInput);
	A.Asc->AbilityLocalInputReleased(JumpInput);
	TestEqual(TEXT("No air attack on the ground"), A.Projectiles.Num(), 0);

	A.Character->LaunchCharacter(FVector(0.f, 0.f, 800.f), false, true);
	F.Tick(0.1f);
	TestTrue(TEXT("Airborne"), A.Character->GetCharacterMovement()->IsFalling());
	A.Asc->AbilityLocalInputPressed(JumpInput);
	A.Asc->AbilityLocalInputReleased(JumpInput);
	if (TestEqual(TEXT("Jump button fires the air attack in the air"), A.Projectiles.Num(), 1) && A.Projectiles[0].IsValid())
	{
		const FVector Direction = A.Projectiles[0]->GetActorForwardVector();
		TestTrue(FString::Printf(TEXT("Air attack fires down (dir z %.2f)"), Direction.Z), Direction.Z < -0.99f);
	}
	return true;
}

#endif
