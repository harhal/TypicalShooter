// Tests for UHealthComponent (player and enemy health: refill or stay depleted) and the player's health setup.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "AbilitySystemComponent.h"
#include "Engine/HitResult.h"
#include "Shooting/GasDamageReceiverComponent.h"
#include "Shooting/HealthAttributeSet.h"
#include "Shooting/HealthComponent.h"
#include "Shooting/ShootingTags.h"
#include "Shooting/ShotDamage.h"
#include "Tests/ShootingTestFixture.h"
#include "TypicalShooterCharacter.h"

namespace EnemiesHealthTest
{
	struct FTarget
	{
		UAbilitySystemComponent* Asc = nullptr;
		UGasDamageReceiverComponent* Receiver = nullptr;
		UHealthComponent* Health = nullptr;
		TSharedRef<int32> Depleted = MakeShared<int32>(0);

		void Hit(const float Amount) const
		{
			FShotDamage Damage;
			Damage.Amounts.Add(ShootingTags::Damage_Type_Physical, Amount);
			Receiver->ReceiveShotDamage(Damage, FHitResult());
		}

		float Value() const { return Asc->GetNumericAttribute(UHealthAttributeSet::GetHealthAttribute()); }
	};

	/** Playing actor with an ASC, a health set, a GAS receiver and a health component (MaxHealth 50). */
	FTarget Spawn(const ShootingTest::FWorldFixture& F, const bool bRefill)
	{
		FTarget T;
		T.Asc = F.SpawnWithComponent<UAbilitySystemComponent>();
		AActor* Actor = T.Asc->GetOwner();
		T.Asc->InitAbilityActorInfo(Actor, Actor);
		T.Asc->AddSpawnedAttribute(NewObject<UHealthAttributeSet>(Actor));
		T.Receiver = NewObject<UGasDamageReceiverComponent>(Actor);
		T.Receiver->RegisterComponent();

		T.Health = NewObject<UHealthComponent>(Actor);
		T.Health->MaxHealth = 50.f;
		T.Health->bRefillWhenDepleted = bRefill;
		T.Health->GetDepletedDelegate().AddLambda([Counter = T.Depleted]() { ++*Counter; });
		T.Health->RegisterComponent(); // the actor already plays: BeginPlay runs here
		return T;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesHealthStartsFullTest, "TypicalShooter.Enemies.Health.StartsFull", ShootingTest::Flags)
bool FEnemiesHealthStartsFullTest::RunTest(const FString& Parameters)
{
	const ShootingTest::FWorldFixture F(true);
	const EnemiesHealthTest::FTarget T = EnemiesHealthTest::Spawn(F, false);
	TestEqual(TEXT("MaxHealth attribute = MaxHealth"), T.Health->GetMaxHealth(), 50.f);
	TestEqual(TEXT("Health starts full"), T.Health->GetHealth(), 50.f);
	T.Hit(20.f);
	TestEqual(TEXT("Damage lowers health"), T.Health->GetHealth(), 30.f);
	TestFalse(TEXT("Not depleted yet"), T.Health->IsDepleted());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesHealthRefillsTest, "TypicalShooter.Enemies.Health.RefillsWhenDepleted", ShootingTest::Flags)
bool FEnemiesHealthRefillsTest::RunTest(const FString& Parameters)
{
	const ShootingTest::FWorldFixture F(true);
	const EnemiesHealthTest::FTarget T = EnemiesHealthTest::Spawn(F, true);
	T.Hit(80.f);
	TestEqual(TEXT("Depleted event fired once"), *T.Depleted, 1);
	TestEqual(TEXT("Defeat counted"), T.Health->GetDefeats(), 1);
	F.Tick(0.1f);
	TestEqual(TEXT("Health refilled to max"), T.Value(), 50.f);
	TestFalse(TEXT("Not depleted after refill"), T.Health->IsDepleted());

	T.Hit(60.f);
	TestEqual(TEXT("Depleted again after the refill"), *T.Depleted, 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesHealthStaysDepletedTest, "TypicalShooter.Enemies.Health.StaysDepleted", ShootingTest::Flags)
bool FEnemiesHealthStaysDepletedTest::RunTest(const FString& Parameters)
{
	const ShootingTest::FWorldFixture F(true);
	const EnemiesHealthTest::FTarget T = EnemiesHealthTest::Spawn(F, false);
	T.Hit(80.f);
	F.Tick(0.1f);
	TestEqual(TEXT("Health stays at zero"), T.Value(), 0.f);
	TestTrue(TEXT("Depleted"), T.Health->IsDepleted());
	T.Hit(10.f);
	TestEqual(TEXT("Depleted event fired only once"), *T.Depleted, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesPlayerHealthTest, "TypicalShooter.Enemies.PlayerHealth.CharacterIsImmortal", ShootingTest::Flags)
bool FEnemiesPlayerHealthTest::RunTest(const FString& Parameters)
{
	const ATypicalShooterCharacter* Character = GetDefault<ATypicalShooterCharacter>();
	const UHealthComponent* Health = Character->FindComponentByClass<UHealthComponent>();
	TestNotNull(TEXT("Player has a health component"), Health);
	TestTrue(TEXT("Player health refills when depleted (immortal for now)"), Health && Health->bRefillWhenDepleted);
	TestNotNull(TEXT("Player receives shot damage through GAS"), Character->FindComponentByClass<UGasDamageReceiverComponent>());

	bool bHasHealthSet = false;
	ForEachObjectWithOuter(Character, [&bHasHealthSet](const UObject* Object) { bHasHealthSet |= Object->IsA<UHealthAttributeSet>(); }, false);
	TestTrue(TEXT("Player owns a health attribute set"), bHasHealthSet);
	return true;
}

#endif
