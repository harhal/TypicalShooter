// Tests for ATargetDummy: GAS health, health label, refill when depleted.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/TextRenderComponent.h"
#include "Engine/HitResult.h"
#include "Shooting/ShootingTags.h"
#include "Shooting/ShotDamage.h"
#include "Shooting/ShotDamageReceiver.h"
#include "Shooting/TargetDummy.h"
#include "Tests/ShootingTestFixture.h"

namespace ShootingTargetDummyTest
{
	void Hit(ATargetDummy* Dummy, const float Physical)
	{
		FShotDamage Damage;
		Damage.Amounts.Add(ShootingTags::Damage_Type_Physical, Physical);
		if (IShotDamageReceiver* Receiver = IShotDamageReceiver::FindOnActor(Dummy))
		{
			Receiver->ReceiveShotDamage(Damage, FHitResult());
		}
	}

	FString Label(const ATargetDummy* Dummy)
	{
		return Dummy->GetHealthText()->Text.ToString();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTargetDummyDamageTest, "TypicalShooter.Shooting.TargetDummy.TakesDamage", ShootingTest::Flags)
bool FTargetDummyDamageTest::RunTest(const FString& Parameters)
{
	const ShootingTest::FWorldFixture F(true);
	ATargetDummy* Dummy = F.World->SpawnActor<ATargetDummy>();

	TestEqual(TEXT("Starts at MaxHealth"), Dummy->GetHealth(), 100.f);
	TestEqual(TEXT("Label shows full health"), ShootingTargetDummyTest::Label(Dummy), FString(TEXT("100 / 100")));
	TestNotNull(TEXT("Dummy has a damage receiver"), IShotDamageReceiver::FindOnActor(Dummy));

	ShootingTargetDummyTest::Hit(Dummy, 30.f);
	TestEqual(TEXT("Hit reduces health"), Dummy->GetHealth(), 70.f);
	TestEqual(TEXT("Damage is accumulated"), Dummy->GetTotalDamageTaken(), 30.f);
	TestEqual(TEXT("Label follows health"), ShootingTargetDummyTest::Label(Dummy), FString(TEXT("70 / 100")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTargetDummyRefillTest, "TypicalShooter.Shooting.TargetDummy.RefillsWhenDepleted", ShootingTest::Flags)
bool FTargetDummyRefillTest::RunTest(const FString& Parameters)
{
	const ShootingTest::FWorldFixture F(true);
	ATargetDummy* Dummy = F.World->SpawnActor<ATargetDummy>();

	ShootingTargetDummyTest::Hit(Dummy, 150.f);
	TestEqual(TEXT("Depletion counted"), Dummy->GetDefeats(), 1);
	TestEqual(TEXT("Only real health loss is counted"), Dummy->GetTotalDamageTaken(), 100.f);

	F.Tick(0.1f);
	TestEqual(TEXT("Health refilled"), Dummy->GetHealth(), 100.f);
	TestEqual(TEXT("Label shows refilled health"), ShootingTargetDummyTest::Label(Dummy), FString(TEXT("100 / 100")));

	ShootingTargetDummyTest::Hit(Dummy, 10.f);
	TestEqual(TEXT("Takes damage again after refill"), Dummy->GetHealth(), 90.f);
	return true;
}

#endif
