// Tests for UAttackAttributeSet: independent attack slots, clamping (R1-R3), resolved stats.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "AbilitySystemComponent.h"
#include "Shooting/AttackAttributeSet.h"
#include "Shooting/ShootingTags.h"
#include "Tests/ShootingTestFixture.h"

namespace ShootingAttributesTest
{
	UAbilitySystemComponent* SpawnAsc(const ShootingTest::FWorldFixture& F)
	{
		UAbilitySystemComponent* Asc = F.SpawnWithComponent<UAbilitySystemComponent>();
		AActor* Actor = Asc->GetOwner();
		Asc->InitAbilityActorInfo(Actor, Actor);
		Asc->AddSpawnedAttribute(NewObject<UAttackAttributeSet>(Actor));
		return Asc;
	}

	void Set(UAbilitySystemComponent* Asc, const EAttackSlot Slot, const EAttackAttribute Attribute, const float Value)
	{
		Asc->SetNumericAttributeBase(UAttackAttributeSet::GetAttribute(Slot, Attribute), Value);
	}

	float Get(const UAbilitySystemComponent* Asc, const EAttackSlot Slot, const EAttackAttribute Attribute)
	{
		return Asc->GetNumericAttribute(UAttackAttributeSet::GetAttribute(Slot, Attribute));
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAttackSlotsIndependentTest, "TypicalShooter.Shooting.Attributes.SlotsIndependent", ShootingTest::Flags)
bool FAttackSlotsIndependentTest::RunTest(const FString& Parameters)
{
	using namespace ShootingAttributesTest;
	const ShootingTest::FWorldFixture F;
	UAbilitySystemComponent* Asc = SpawnAsc(F);

	TestTrue(TEXT("Every slot parameter has an attribute"),
		UAttackAttributeSet::GetAttribute(EAttackSlot::Alternate, EAttackAttribute::DamageRadiation).IsValid());
	TestFalse(TEXT("Slots use different attributes"),
		UAttackAttributeSet::GetAttribute(EAttackSlot::Primary, EAttackAttribute::ShotInterval)
		== UAttackAttributeSet::GetAttribute(EAttackSlot::Alternate, EAttackAttribute::ShotInterval));

	Set(Asc, EAttackSlot::Primary, EAttackAttribute::ShotInterval, 2.f);
	Set(Asc, EAttackSlot::Alternate, EAttackAttribute::ShotInterval, 5.f);
	TestEqual(TEXT("Primary keeps its value"), Get(Asc, EAttackSlot::Primary, EAttackAttribute::ShotInterval), 2.f);
	TestEqual(TEXT("Alternate keeps its value"), Get(Asc, EAttackSlot::Alternate, EAttackAttribute::ShotInterval), 5.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAttackClampTest, "TypicalShooter.Shooting.Attributes.Clamp", ShootingTest::Flags)
bool FAttackClampTest::RunTest(const FString& Parameters)
{
	using namespace ShootingAttributesTest;
	const ShootingTest::FWorldFixture F;
	UAbilitySystemComponent* Asc = SpawnAsc(F);
	constexpr EAttackSlot S = EAttackSlot::Primary;

	Set(Asc, S, EAttackAttribute::ShotsPerBurst, 0.f);
	TestEqual(TEXT("At least one shot per burst"), Get(Asc, S, EAttackAttribute::ShotsPerBurst), 1.f);
	Set(Asc, S, EAttackAttribute::DamageLoss, 1.5f);
	TestEqual(TEXT("Damage loss is at most 1"), Get(Asc, S, EAttackAttribute::DamageLoss), 1.f);
	Set(Asc, S, EAttackAttribute::DamageAcid, -10.f);
	TestEqual(TEXT("Damage is not negative"), Get(Asc, S, EAttackAttribute::DamageAcid), 0.f);
	Set(Asc, S, EAttackAttribute::ShotInterval, -1.f);
	TestEqual(TEXT("Intervals are not negative"), Get(Asc, S, EAttackAttribute::ShotInterval), 0.f);
	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAttackReadStatsTest, "TypicalShooter.Shooting.Attributes.ReadStats", ShootingTest::Flags)
bool FAttackReadStatsTest::RunTest(const FString& Parameters)
{
	using namespace ShootingAttributesTest;
	const ShootingTest::FWorldFixture F;
	UAbilitySystemComponent* Asc = SpawnAsc(F);
	constexpr EAttackSlot S = EAttackSlot::Primary;

	Set(Asc, S, EAttackAttribute::FireStartDistance, 500.f);
	Set(Asc, S, EAttackAttribute::FireEndDistance, 200.f);
	Set(Asc, S, EAttackAttribute::DamagePhysical, 10.f);
	Set(Asc, S, EAttackAttribute::DamagePlasma, 4.f);
	Set(Asc, EAttackSlot::Alternate, EAttackAttribute::DamagePhysical, 99.f);

	const FAttackStats Stats = UAttackAttributeSet::ReadStats(*Asc, S);
	TestEqual(TEXT("Fire end is never before fire start"), Stats.FireEndDistance, 500.f);
	TestEqual(TEXT("Physical damage from its attribute"), Stats.Damage.Get(ShootingTags::Damage_Type_Physical), 10.f);
	TestEqual(TEXT("Plasma damage from its attribute"), Stats.Damage.Get(ShootingTags::Damage_Type_Plasma), 4.f);
	TestEqual(TEXT("Total damage of the slot only"), Stats.Damage.GetTotal(), 14.f);
	TestEqual(TEXT("Defaults: projectile speed"), Stats.ProjectileSpeed, 5000.f);
	TestEqual(TEXT("Defaults: damage loss stops at the first hit"), Stats.DamageLoss, 1.f);
	return true;
}

#endif
