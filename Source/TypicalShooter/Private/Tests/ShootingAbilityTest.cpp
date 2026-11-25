// Tests for UAttackGameplayAbility: a shooter actor with an ASC fires at target dummies in a ticking world.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "AbilitySystemComponent.h"
#include "Shooting/AttackAttributeSet.h"
#include "Shooting/AttackGameplayAbility.h"
#include "Shooting/HealthAttributeSet.h"
#include "Shooting/TargetDummy.h"
#include "Tests/ShootingTestFixture.h"

namespace ShootingAbilityTest
{
	/** Actor at the origin looking along +X, with attack attributes and one attack ability. */
	struct FShooter
	{
		UAbilitySystemComponent* Asc = nullptr;
		FGameplayAbilitySpecHandle Handle;

		FShooter(const ShootingTest::FWorldFixture& F, const EAttackSlot Slot, const FShotPattern& Pattern = FShotPattern())
		{
			Asc = F.SpawnWithComponent<UAbilitySystemComponent>();
			AActor* Actor = Asc->GetOwner();
			Asc->InitAbilityActorInfo(Actor, Actor);
			Asc->AddSpawnedAttribute(NewObject<UAttackAttributeSet>(Actor));
			Handle = Asc->GiveAbility(FGameplayAbilitySpec(UAttackGameplayAbility::StaticClass()));
			UAttackGameplayAbility* Ability = Cast<UAttackGameplayAbility>(Spec()->GetPrimaryInstance());
			Ability->Slot = Slot;
			Ability->Pattern = Pattern;
		}

		FGameplayAbilitySpec* Spec() const { return Asc->FindAbilitySpecFromHandle(Handle); }
		bool Activate() const { return Asc->TryActivateAbility(Handle); }
		bool IsActive() const { return Spec()->IsActive(); }
		void SetHeld(const bool bHeld) const { Spec()->InputPressed = bHeld; }

		void Set(const EAttackSlot Slot, const EAttackAttribute Attribute, const float Value) const
		{
			Asc->SetNumericAttributeBase(UAttackAttributeSet::GetAttribute(Slot, Attribute), Value);
		}

		float Get(const EAttackSlot Slot, const EAttackAttribute Attribute) const
		{
			return Asc->GetNumericAttribute(UAttackAttributeSet::GetAttribute(Slot, Attribute));
		}

		/** Clip of Ammo rounds, Shots per burst, physical damage per hit. */
		void Setup(const EAttackSlot Slot, const float Ammo, const float Shots, const float ShotInterval, const float BurstInterval,
			const float Damage) const
		{
			Set(Slot, EAttackAttribute::ClipSize, 30.f);
			Set(Slot, EAttackAttribute::Ammo, Ammo);
			Set(Slot, EAttackAttribute::ShotsPerBurst, Shots);
			Set(Slot, EAttackAttribute::ShotInterval, ShotInterval);
			Set(Slot, EAttackAttribute::BurstInterval, BurstInterval);
			Set(Slot, EAttackAttribute::DamagePhysical, Damage);
			Set(Slot, EAttackAttribute::ProjectileSpeed, 10000.f);
		}
	};

	ATargetDummy* SpawnDummy(const ShootingTest::FWorldFixture& F)
	{
		ATargetDummy* Dummy = F.World->SpawnActor<ATargetDummy>(FVector(500.f, 0.f, 0.f), FRotator::ZeroRotator);
		Dummy->MaxHealth = 1000.f; // effects are applied at BeginPlay already, set health explicitly
		Dummy->GetAbilitySystemComponent()->SetNumericAttributeBase(UHealthAttributeSet::GetMaxHealthAttribute(), 1000.f);
		Dummy->GetAbilitySystemComponent()->SetNumericAttributeBase(UHealthAttributeSet::GetHealthAttribute(), 1000.f);
		return Dummy;
	}

	float Lost(const ATargetDummy* Dummy)
	{
		return Dummy->GetTotalDamageTaken();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAttackSingleShotTest, "TypicalShooter.Shooting.Ability.SingleShot", ShootingTest::Flags)
bool FAttackSingleShotTest::RunTest(const FString& Parameters)
{
	using namespace ShootingAbilityTest;
	const ShootingTest::FWorldFixture F(true);
	const ATargetDummy* Dummy = SpawnDummy(F);
	const FShooter Shooter(F, EAttackSlot::Primary);
	Shooter.Setup(EAttackSlot::Primary, 10.f, 1.f, 0.f, 0.f, 10.f);

	TestTrue(TEXT("Activates"), Shooter.Activate());
	TestEqual(TEXT("One round spent"), Shooter.Get(EAttackSlot::Primary, EAttackAttribute::Ammo), 9.f);
	F.Tick(0.2f);
	TestEqual(TEXT("The projectile hits the dummy"), Lost(Dummy), 10.f);
	TestFalse(TEXT("Ability ends after the burst"), Shooter.IsActive());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAttackBurstTest, "TypicalShooter.Shooting.Ability.Burst", ShootingTest::Flags)
bool FAttackBurstTest::RunTest(const FString& Parameters)
{
	using namespace ShootingAbilityTest;
	const ShootingTest::FWorldFixture F(true);
	const ATargetDummy* Dummy = SpawnDummy(F);
	const FShooter Shooter(F, EAttackSlot::Primary);
	Shooter.Setup(EAttackSlot::Primary, 10.f, 3.f, 0.1f, 0.5f, 10.f);

	Shooter.Activate();
	TestEqual(TEXT("First shot is immediate"), Shooter.Get(EAttackSlot::Primary, EAttackAttribute::Ammo), 9.f);
	F.Tick(0.05f);
	TestEqual(TEXT("Next shot waits for the shot interval"), Shooter.Get(EAttackSlot::Primary, EAttackAttribute::Ammo), 9.f);
	F.Tick(0.25f);
	TestEqual(TEXT("Whole burst fired"), Shooter.Get(EAttackSlot::Primary, EAttackAttribute::Ammo), 7.f);
	TestTrue(TEXT("Ability waits out the burst interval"), Shooter.IsActive());
	F.Tick(0.6f);
	TestEqual(TEXT("All burst projectiles hit"), Lost(Dummy), 30.f);
	TestFalse(TEXT("Ability ends after the burst interval when not held"), Shooter.IsActive());
	TestEqual(TEXT("No extra burst without input"), Shooter.Get(EAttackSlot::Primary, EAttackAttribute::Ammo), 7.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAttackHoldTest, "TypicalShooter.Shooting.Ability.HoldRepeatsBursts", ShootingTest::Flags)
bool FAttackHoldTest::RunTest(const FString& Parameters)
{
	using namespace ShootingAbilityTest;
	const ShootingTest::FWorldFixture F(true);
	SpawnDummy(F);
	const FShooter Shooter(F, EAttackSlot::Primary);
	Shooter.Setup(EAttackSlot::Primary, 20.f, 2.f, 0.1f, 0.3f, 1.f);

	Shooter.SetHeld(true);
	Shooter.Activate();
	F.Tick(0.65f); // bursts at 0 and 0.4 (= 0.1 + 0.3), next one at 0.8
	TestEqual(TEXT("Held input repeats bursts after the burst interval"), Shooter.Get(EAttackSlot::Primary, EAttackAttribute::Ammo), 16.f);

	Shooter.SetHeld(false);
	F.Tick(1.f);
	TestEqual(TEXT("Releasing stops further bursts"), Shooter.Get(EAttackSlot::Primary, EAttackAttribute::Ammo), 16.f);
	TestFalse(TEXT("Ability ended"), Shooter.IsActive());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAttackEmptyClipTest, "TypicalShooter.Shooting.Ability.BurstStopsWhenEmpty", ShootingTest::Flags)
bool FAttackEmptyClipTest::RunTest(const FString& Parameters)
{
	using namespace ShootingAbilityTest;
	const ShootingTest::FWorldFixture F(true);
	const ATargetDummy* Dummy = SpawnDummy(F);
	const FShooter Shooter(F, EAttackSlot::Primary);
	Shooter.Setup(EAttackSlot::Primary, 2.f, 5.f, 0.05f, 0.f, 10.f);

	Shooter.Activate();
	F.Tick(0.5f);
	TestEqual(TEXT("Clip emptied"), Shooter.Get(EAttackSlot::Primary, EAttackAttribute::Ammo), 0.f);
	TestEqual(TEXT("Only the loaded rounds were fired"), Lost(Dummy), 20.f);
	TestFalse(TEXT("Burst stops on an empty clip"), Shooter.IsActive());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAttackPatternTest, "TypicalShooter.Shooting.Ability.PatternProjectiles", ShootingTest::Flags)
bool FAttackPatternTest::RunTest(const FString& Parameters)
{
	using namespace ShootingAbilityTest;
	const ShootingTest::FWorldFixture F(true);
	const ATargetDummy* Dummy = SpawnDummy(F);
	FShotPattern Pattern;
	Pattern.Offsets = { FVector2D(0.f, 0.f), FVector2D(1.f, 0.f), FVector2D(-1.f, 0.f) };
	const FShooter Shooter(F, EAttackSlot::Primary, Pattern);
	Shooter.Setup(EAttackSlot::Primary, 10.f, 1.f, 0.f, 0.f, 5.f);

	Shooter.Activate();
	F.Tick(0.2f);
	TestEqual(TEXT("One round per shot, whatever the projectile count"), Shooter.Get(EAttackSlot::Primary, EAttackAttribute::Ammo), 9.f);
	TestEqual(TEXT("Every pattern point fires a projectile"), Lost(Dummy), 15.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAttackAlternateSlotTest, "TypicalShooter.Shooting.Ability.AlternateSlot", ShootingTest::Flags)
bool FAttackAlternateSlotTest::RunTest(const FString& Parameters)
{
	using namespace ShootingAbilityTest;
	const ShootingTest::FWorldFixture F(true);
	const ATargetDummy* Dummy = SpawnDummy(F);
	const FShooter Shooter(F, EAttackSlot::Alternate);
	Shooter.Setup(EAttackSlot::Primary, 10.f, 1.f, 0.f, 0.f, 10.f);
	Shooter.Setup(EAttackSlot::Alternate, 5.f, 1.f, 0.f, 0.f, 3.f);

	Shooter.Activate();
	F.Tick(0.2f);
	TestEqual(TEXT("Alternate ammo spent"), Shooter.Get(EAttackSlot::Alternate, EAttackAttribute::Ammo), 4.f);
	TestEqual(TEXT("Primary ammo untouched"), Shooter.Get(EAttackSlot::Primary, EAttackAttribute::Ammo), 10.f);
	TestEqual(TEXT("Alternate damage applied"), Lost(Dummy), 3.f);
	return true;
}

#endif
