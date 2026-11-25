// Receiver abilities: the active receiver of a slot grants its ability with the slot's input; the air attack works
// only while airborne and fires down (power grid R5, R5b, R6a). Ticking world with a floor and an AI character.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "AbilitySystemComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InventoryComponent.h"
#include "PowerGrid/PowerGridComponent.h"
#include "PowerGrid/PowerGridFragment.h"
#include "PowerGrid/ReceiverAbilitiesComponent.h"
#include "Shooting/AttackAttributeSet.h"
#include "Shooting/AttackGameplayAbility.h"
#include "Shooting/ShotProjectile.h"
#include "Tests/ShootingTestFixture.h"

namespace PowerGridReceiverAbilitiesTest
{
	FInventoryItemManifest Manifest(const EPowerItemType Type)
	{
		FPowerGrid_ItemFragment Fragment;
		Fragment.ItemType = Type;
		Fragment.Receiver.Ability = UAttackGameplayAbility::StaticClass();
		FInventoryItemManifest Result;
		Result.Fragments.Add(TInstancedStruct<FItemFragment>::Make<FPowerGrid_ItemFragment>(Fragment));
		return Result;
	}

	/** AI character on a floor with an ASC, attack attributes, inventory, power grid and receiver abilities. */
	struct FArmed
	{
		ACharacter* Character = nullptr;
		UAbilitySystemComponent* Asc = nullptr;
		UInventoryComponent* Inventory = nullptr;
		TArray<TWeakObjectPtr<AShotProjectile>> Projectiles;

		explicit FArmed(const ShootingTest::FWorldFixture& F)
		{
			AActor* Floor = F.World->SpawnActor<AActor>();
			UBoxComponent* Box = NewObject<UBoxComponent>(Floor);
			Box->SetBoxExtent(FVector(2000.f, 2000.f, 10.f));
			Box->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
			Floor->SetRootComponent(Box);
			Box->RegisterComponent();
			Box->SetWorldLocation(FVector(0.f, 0.f, -10.f));

			Character = F.World->SpawnActor<ACharacter>(FVector(0.f, 0.f, 150.f), FRotator::ZeroRotator);
			Character->SpawnDefaultController();
			Asc = NewObject<UAbilitySystemComponent>(Character);
			Asc->RegisterComponent();
			Asc->InitAbilityActorInfo(Character, Character);
			Asc->AddSpawnedAttribute(NewObject<UAttackAttributeSet>(Character));
			Inventory = NewObject<UInventoryComponent>(Character);
			Inventory->RegisterComponent();
			NewObject<UPowerGridComponent>(Character)->RegisterComponent();
			NewObject<UReceiverAbilitiesComponent>(Character)->RegisterComponent();

			F.World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateLambda([this](AActor* Spawned)
			{
				if (AShotProjectile* Projectile = Cast<AShotProjectile>(Spawned))
				{
					Projectiles.Add(Projectile);
				}
			}));
		}

		/** Specs granted for receivers, by input id. */
		const FGameplayAbilitySpec* FindSpec(const EAttackInputID Input) const
		{
			for (const FGameplayAbilitySpec& Spec : Asc->GetActivatableAbilities())
			{
				if (Spec.InputID == static_cast<int32>(Input))
				{
					return &Spec;
				}
			}
			return nullptr;
		}

		TOptional<EAttackSlot> SlotOf(const FGameplayAbilitySpec* Spec) const
		{
			const UAttackGameplayAbility* Ability = Spec ? Cast<UAttackGameplayAbility>(Spec->GetPrimaryInstance()) : nullptr;
			return Ability ? Ability->Slot : TOptional<EAttackSlot>();
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FReceiverGrantsAbilityTest, "TypicalShooter.PowerGrid.Receivers.GrantsSlotAbility", ShootingTest::Flags)
bool FReceiverGrantsAbilityTest::RunTest(const FString& Parameters)
{
	using namespace PowerGridReceiverAbilitiesTest;
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
	using namespace PowerGridReceiverAbilitiesTest;
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
