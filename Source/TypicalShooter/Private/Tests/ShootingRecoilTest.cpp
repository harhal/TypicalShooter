// Recoil: every shot pushes the shooting character opposite to the shot by the slot's Recoil (power grid R5c);
// the air attack fires down, so its recoil pushes up (a double jump).

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "GameFramework/CharacterMovementComponent.h"
#include "Tests/PowerGridTestFixture.h"

namespace ShootingRecoilTest
{
	/** Gives the character an attack ability of Slot with Recoil and no damage. */
	FGameplayAbilitySpecHandle Arm(const PowerGridTest::FArmed& A, const EAttackSlot Slot, const float Recoil)
	{
		const FGameplayAbilitySpecHandle Handle = A.Asc->GiveAbility(FGameplayAbilitySpec(UAttackGameplayAbility::StaticClass()));
		Cast<UAttackGameplayAbility>(A.Asc->FindAbilitySpecFromHandle(Handle)->GetPrimaryInstance())->Slot = Slot;
		A.Asc->SetNumericAttributeBase(UAttackAttributeSet::GetAttribute(Slot, EAttackAttribute::Recoil), Recoil);
		return Handle;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FShootingRecoilTest, "TypicalShooter.Shooting.Ability.Recoil", ShootingTest::Flags)
bool FShootingRecoilTest::RunTest(const FString& Parameters)
{
	using namespace ShootingRecoilTest;
	const ShootingTest::FWorldFixture F(true);
	const PowerGridTest::FArmed A(F);
	UCharacterMovementComponent* Movement = A.Character->GetCharacterMovement();
	const FGameplayAbilitySpecHandle Primary = Arm(A, EAttackSlot::Primary, 600.f);
	const FGameplayAbilitySpecHandle Air = Arm(A, EAttackSlot::Air, 900.f);
	F.Tick(1.f);

	// The AI character looks along +X: the primary shot goes forward, the recoil pushes back.
	TestTrue(TEXT("Primary shot fired"), A.Asc->TryActivateAbility(Primary));
	F.Tick(1.f / 60.f);
	TestTrue(FString::Printf(TEXT("Recoil pushes back from the shot (vx %.0f)"), Movement->Velocity.X), Movement->Velocity.X < -300.f);

	F.Tick(1.f);
	A.Character->LaunchCharacter(FVector(0.f, 0.f, 300.f), false, true);
	F.Tick(0.2f);
	const float FallingSpeed = Movement->Velocity.Z;
	TestTrue(TEXT("Air attack fired while airborne"), A.Asc->TryActivateAbility(Air));
	F.Tick(1.f / 60.f);
	TestTrue(FString::Printf(TEXT("Air attack recoil pushes up: a double jump (vz %.0f -> %.0f)"), FallingSpeed, Movement->Velocity.Z),
		Movement->Velocity.Z > FallingSpeed + 800.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FShootingNoRecoilTest, "TypicalShooter.Shooting.Ability.NoRecoilByDefault", ShootingTest::Flags)
bool FShootingNoRecoilTest::RunTest(const FString& Parameters)
{
	using namespace ShootingRecoilTest;
	const ShootingTest::FWorldFixture F(true);
	const PowerGridTest::FArmed A(F);
	const FGameplayAbilitySpecHandle Primary = Arm(A, EAttackSlot::Primary, 0.f);
	F.Tick(1.f);
	TestTrue(TEXT("Shot fired"), A.Asc->TryActivateAbility(Primary));
	F.Tick(1.f / 60.f);
	TestTrue(TEXT("No recoil: the character stays put"), A.Character->GetCharacterMovement()->Velocity.IsNearlyZero(1.f));
	return true;
}

#endif
