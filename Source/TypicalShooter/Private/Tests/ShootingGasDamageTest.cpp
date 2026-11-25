// Tests for GAS damage: health attribute set, shot damage effect and GAS damage receiver.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "AbilitySystemComponent.h"
#include "Engine/HitResult.h"
#include "Shooting/GasDamageReceiverComponent.h"
#include "Shooting/HealthAttributeSet.h"
#include "Shooting/ShootingTags.h"
#include "Shooting/ShotDamage.h"
#include "Tests/ShootingTestFixture.h"

namespace ShootingGasDamageTest
{
	struct FGasTarget
	{
		UAbilitySystemComponent* Asc = nullptr;
		UGasDamageReceiverComponent* Receiver = nullptr;
	};

	/** Plain actor with an ASC, a health set (Health = MaxHealth = 100) and a GAS receiver. */
	FGasTarget SpawnGasTarget(const ShootingTest::FWorldFixture& F)
	{
		FGasTarget Target;
		Target.Asc = F.SpawnWithComponent<UAbilitySystemComponent>();
		AActor* Actor = Target.Asc->GetOwner();
		Target.Asc->InitAbilityActorInfo(Actor, Actor);
		Target.Asc->AddSpawnedAttribute(NewObject<UHealthAttributeSet>(Actor));
		Target.Asc->SetNumericAttributeBase(UHealthAttributeSet::GetMaxHealthAttribute(), 100.f);
		Target.Asc->SetNumericAttributeBase(UHealthAttributeSet::GetHealthAttribute(), 100.f);

		Target.Receiver = NewObject<UGasDamageReceiverComponent>(Actor);
		Target.Receiver->RegisterComponent();
		return Target;
	}

	float Health(const FGasTarget& Target)
	{
		return Target.Asc->GetNumericAttribute(UHealthAttributeSet::GetHealthAttribute());
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHealthClampTest, "TypicalShooter.Shooting.Damage.HealthClamp", ShootingTest::Flags)
bool FHealthClampTest::RunTest(const FString& Parameters)
{
	ShootingTest::FWorldFixture F;
	const ShootingGasDamageTest::FGasTarget Target = ShootingGasDamageTest::SpawnGasTarget(F);

	Target.Asc->SetNumericAttributeBase(UHealthAttributeSet::GetHealthAttribute(), 150.f);
	TestEqual(TEXT("Health clamps to MaxHealth"), ShootingGasDamageTest::Health(Target), 100.f);
	Target.Asc->SetNumericAttributeBase(UHealthAttributeSet::GetHealthAttribute(), -5.f);
	TestEqual(TEXT("Health clamps to zero"), ShootingGasDamageTest::Health(Target), 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGasReceiverTest, "TypicalShooter.Shooting.Damage.GasReceiver", ShootingTest::Flags)
bool FGasReceiverTest::RunTest(const FString& Parameters)
{
	ShootingTest::FWorldFixture F;
	const ShootingGasDamageTest::FGasTarget Target = ShootingGasDamageTest::SpawnGasTarget(F);
	TestTrue(TEXT("GAS receiver is found on its actor"),
		IShotDamageReceiver::FindOnActor(Target.Asc->GetOwner()) == static_cast<IShotDamageReceiver*>(Target.Receiver));

	FShotDamage Damage;
	Damage.Amounts.Add(ShootingTags::Damage_Type_Physical, 30.f);
	Damage.Amounts.Add(ShootingTags::Damage_Type_Radiation, 20.f);
	Target.Receiver->ReceiveShotDamage(Damage, FHitResult());
	TestEqual(TEXT("Every damage type reduces health"), ShootingGasDamageTest::Health(Target), 50.f);
	TestEqual(TEXT("IncomingDamage is reset after execution"),
		Target.Asc->GetNumericAttribute(UHealthAttributeSet::GetIncomingDamageAttribute()), 0.f);

	Target.Receiver->ReceiveShotDamage(Damage, FHitResult());
	Target.Receiver->ReceiveShotDamage(Damage, FHitResult());
	TestEqual(TEXT("Health does not drop below zero"), ShootingGasDamageTest::Health(Target), 0.f);
	return true;
}

#endif
