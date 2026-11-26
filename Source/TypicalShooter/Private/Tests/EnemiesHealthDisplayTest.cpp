// Tests for the HUD health text of the player.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "AbilitySystemComponent.h"
#include "Shooting/HealthAttributeSet.h"
#include "Shooting/HealthDisplay.h"
#include "Tests/ShootingTestFixture.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesHealthTextTest, "TypicalShooter.Enemies.UI.HealthText", ShootingTest::Flags)
bool FEnemiesHealthTextTest::RunTest(const FString& Parameters)
{
	const ShootingTest::FWorldFixture F;
	UAbilitySystemComponent* Asc = F.SpawnWithComponent<UAbilitySystemComponent>();
	Asc->InitAbilityActorInfo(Asc->GetOwner(), Asc->GetOwner());
	TestEqual(TEXT("No health set: nothing to show"), HealthDisplay::GetHealthText(*Asc).ToString(), FString());

	Asc->AddSpawnedAttribute(NewObject<UHealthAttributeSet>(Asc->GetOwner()));
	Asc->SetNumericAttributeBase(UHealthAttributeSet::GetMaxHealthAttribute(), 100.f);
	Asc->SetNumericAttributeBase(UHealthAttributeSet::GetHealthAttribute(), 72.4f);
	TestEqual(TEXT("Current / max, fractions rounded up"), HealthDisplay::GetHealthText(*Asc).ToString(), FString(TEXT("Health: 73 / 100")));
	return true;
}

#endif
