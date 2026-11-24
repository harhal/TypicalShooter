// Tests for shot damage: FShotDamage and damage receivers.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/HitResult.h"
#include "Shooting/ShootingTags.h"
#include "Shooting/ShotDamage.h"
#include "Shooting/ShotDamageReceiver.h"
#include "Shooting/SimpleDamageReceiverComponent.h"
#include "Tests/ShootingTestFixture.h"

namespace ShootingDamageTest
{
	FShotDamage MakeDamage(const float Physical, const float Acid)
	{
		FShotDamage Damage;
		Damage.Amounts.Add(ShootingTags::Damage_Type_Physical, Physical);
		Damage.Amounts.Add(ShootingTags::Damage_Type_Acid, Acid);
		return Damage;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FShotDamageAmountsTest, "TypicalShooter.Shooting.Damage.ShotDamageAmounts", ShootingTest::Flags)
bool FShotDamageAmountsTest::RunTest(const FString& Parameters)
{
	FShotDamage Damage = ShootingDamageTest::MakeDamage(30.f, 20.f);
	TestEqual(TEXT("Get returns the amount of a type"), Damage.Get(ShootingTags::Damage_Type_Acid), 20.f);
	TestEqual(TEXT("Get of an absent type is 0"), Damage.Get(ShootingTags::Damage_Type_Ice), 0.f);
	TestEqual(TEXT("Total sums all types"), Damage.GetTotal(), 50.f);
	TestTrue(TEXT("Has damage"), Damage.HasDamage());

	Damage.Scale(0.5f);
	TestEqual(TEXT("Scale multiplies every type"), Damage.Get(ShootingTags::Damage_Type_Physical), 15.f);
	TestEqual(TEXT("Scaled total"), Damage.GetTotal(), 25.f);

	Damage.Scale(-1.f);
	TestEqual(TEXT("Negative scale clamps to zero"), Damage.GetTotal(), 0.f);
	TestFalse(TEXT("No damage left"), Damage.HasDamage());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSimpleReceiverTest, "TypicalShooter.Shooting.Damage.SimpleReceiver", ShootingTest::Flags)
bool FSimpleReceiverTest::RunTest(const FString& Parameters)
{
	ShootingTest::FWorldFixture F;
	USimpleDamageReceiverComponent* Receiver = F.SpawnWithComponent<USimpleDamageReceiverComponent>(
		[](USimpleDamageReceiverComponent& C) { C.MaxDurability = 100.f; });

	TestEqual(TEXT("Starts at max durability"), Receiver->GetDurability(), 100.f);
	TestTrue(TEXT("Receiver is found on its actor"),
		IShotDamageReceiver::FindOnActor(Receiver->GetOwner()) == static_cast<IShotDamageReceiver*>(Receiver));
	TestNull(TEXT("No receiver on a plain actor"), IShotDamageReceiver::FindOnActor(F.World->SpawnActor<AActor>()));

	int32 NumDepleted = 0;
	Receiver->GetDepletedDelegate().AddLambda([&NumDepleted]() { ++NumDepleted; });

	Receiver->ReceiveShotDamage(ShootingDamageTest::MakeDamage(30.f, 20.f), FHitResult());
	TestEqual(TEXT("All damage types reduce durability"), Receiver->GetDurability(), 50.f);
	TestFalse(TEXT("Not depleted yet"), Receiver->IsDepleted());

	Receiver->ReceiveShotDamage(ShootingDamageTest::MakeDamage(60.f, 0.f), FHitResult());
	TestEqual(TEXT("Durability clamps at zero"), Receiver->GetDurability(), 0.f);
	TestTrue(TEXT("Depleted"), Receiver->IsDepleted());

	Receiver->ReceiveShotDamage(ShootingDamageTest::MakeDamage(10.f, 0.f), FHitResult());
	TestEqual(TEXT("Depleted event fires once"), NumDepleted, 1);
	TestFalse(TEXT("Owner kept by default"), Receiver->GetOwner()->IsActorBeingDestroyed());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSimpleReceiverDestroyTest, "TypicalShooter.Shooting.Damage.SimpleReceiverDestroysOwner", ShootingTest::Flags)
bool FSimpleReceiverDestroyTest::RunTest(const FString& Parameters)
{
	ShootingTest::FWorldFixture F;
	USimpleDamageReceiverComponent* Receiver = F.SpawnWithComponent<USimpleDamageReceiverComponent>(
		[](USimpleDamageReceiverComponent& C)
		{
			C.MaxDurability = 10.f;
			C.bDestroyOwnerOnDepleted = true;
		});
	AActor* Owner = Receiver->GetOwner();

	Receiver->ReceiveShotDamage(ShootingDamageTest::MakeDamage(10.f, 0.f), FHitResult());
	TestTrue(TEXT("Owner destroyed on depletion"), !IsValid(Owner) || Owner->IsActorBeingDestroyed());
	return true;
}

#endif
