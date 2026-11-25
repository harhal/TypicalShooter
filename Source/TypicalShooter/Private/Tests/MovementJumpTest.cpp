// Ground jump as a GAS ability with jump power / air control attributes driving the character movement
// (power grid R6c). Ticking game world with a floor, no PIE.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "AbilitySystemComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Movement/JumpGameplayAbility.h"
#include "Movement/MovementAttributeSet.h"
#include "Movement/MovementAttributesComponent.h"
#include "Tests/ShootingTestFixture.h"
#include "TypicalShooterCharacter.h"

namespace MovementJumpTest
{
	constexpr int32 JumpInputId = 7;

	/** AI-controlled character above a large floor, with an ASC, movement attributes and the jump ability. */
	struct FJumper
	{
		ACharacter* Character = nullptr;
		UAbilitySystemComponent* Asc = nullptr;
		FGameplayAbilitySpecHandle Jump;

		explicit FJumper(const ShootingTest::FWorldFixture& F)
		{
			AActor* Floor = F.World->SpawnActor<AActor>(FVector(0.f, 0.f, -10.f), FRotator::ZeroRotator);
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
			Asc->AddSpawnedAttribute(NewObject<UMovementAttributeSet>(Character));
			NewObject<UMovementAttributesComponent>(Character)->RegisterComponent();
			Jump = Asc->GiveAbility(FGameplayAbilitySpec(UJumpGameplayAbility::StaticClass(), 1, JumpInputId));
		}

		UCharacterMovementComponent* Movement() const { return Character->GetCharacterMovement(); }
		float Attribute(const FGameplayAttribute& Attribute) const { return Asc->GetNumericAttribute(Attribute); }
		void SetAttribute(const FGameplayAttribute& Attribute, const float Value) const { Asc->SetNumericAttributeBase(Attribute, Value); }
		bool Press() const { Asc->AbilityLocalInputPressed(JumpInputId); return Asc->FindAbilitySpecFromHandle(Jump)->IsActive(); }
		void Release() const { Asc->AbilityLocalInputReleased(JumpInputId); }
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMovementAttributesTest, "TypicalShooter.Movement.AttributesDriveMovement", ShootingTest::Flags)
bool FMovementAttributesTest::RunTest(const FString& Parameters)
{
	const ShootingTest::FWorldFixture F(true);
	const MovementJumpTest::FJumper J(F);
	const float DefaultJump = GetDefault<UCharacterMovementComponent>()->JumpZVelocity;

	TestEqual(TEXT("Jump power starts from the movement component"), J.Attribute(UMovementAttributeSet::GetJumpVelocityAttribute()), DefaultJump);
	TestEqual(TEXT("Air control starts from the movement component"),
		J.Attribute(UMovementAttributeSet::GetAirControlAttribute()), GetDefault<UCharacterMovementComponent>()->AirControl);

	J.SetAttribute(UMovementAttributeSet::GetJumpVelocityAttribute(), 1000.f);
	J.SetAttribute(UMovementAttributeSet::GetAirControlAttribute(), 0.8f);
	TestEqual(TEXT("Jump power drives JumpZVelocity"), J.Movement()->JumpZVelocity, 1000.f);
	TestEqual(TEXT("Air control drives AirControl"), J.Movement()->AirControl, 0.8f);

	J.SetAttribute(UMovementAttributeSet::GetJumpVelocityAttribute(), -50.f);
	TestEqual(TEXT("Jump power is not negative"), J.Attribute(UMovementAttributeSet::GetJumpVelocityAttribute()), 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMovementJumpAbilityTest, "TypicalShooter.Movement.JumpAbility", ShootingTest::Flags)
bool FMovementJumpAbilityTest::RunTest(const FString& Parameters)
{
	const ShootingTest::FWorldFixture F(true);
	const MovementJumpTest::FJumper J(F);
	J.SetAttribute(UMovementAttributeSet::GetJumpVelocityAttribute(), 1000.f);

	F.Tick(1.f);
	if (!TestTrue(TEXT("Character stands on the floor"), J.Movement()->IsMovingOnGround()))
	{
		return false;
	}

	TestTrue(TEXT("Jump button activates the jump ability on the ground"), J.Press());
	F.Tick(0.05f);
	TestTrue(TEXT("Character is in the air"), J.Movement()->IsFalling());
	TestTrue(FString::Printf(TEXT("Jumps with the jump power attribute (vz %.0f)"), J.Movement()->Velocity.Z),
		J.Movement()->Velocity.Z > 900.f);

	J.Release();
	TestFalse(TEXT("Releasing ends the ability"), J.Asc->FindAbilitySpecFromHandle(J.Jump)->IsActive());
	TestFalse(TEXT("No second jump in the air (the jump button is for the air attack there)"), J.Press());
	J.Release();

	F.Tick(3.f);
	TestTrue(TEXT("Landed again"), J.Movement()->IsMovingOnGround());
	TestTrue(TEXT("Can jump again after landing"), J.Press());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMovementOnCharacterTest, "TypicalShooter.Movement.OnCharacter", ShootingTest::Flags)
bool FMovementOnCharacterTest::RunTest(const FString& Parameters)
{
	// The jump key itself is covered end-to-end by TypicalShooter.Inventory.UIFlow (SpaceBar jumps in PIE).
	const ATypicalShooterCharacter* Character = GetDefault<ATypicalShooterCharacter>();
	TestNotNull(TEXT("Character mirrors movement attributes"), Character->FindComponentByClass<UMovementAttributesComponent>());
	return true;
}

#endif
