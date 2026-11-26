// Tests for the attack token count of an enemy group: round(Fraction * size), clamped to [Min, Max] and to the group size.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Enemies/Coordination/EnemyCoordinationSettings.h"
#include "Enemies/Coordination/EnemyTokenCount.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesCoordinationTokenCountTest, "TypicalShooter.Enemies.Coordination.TokenCount",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FEnemiesCoordinationTokenCountTest::RunTest(const FString& Parameters)
{
	FEnemyTokenCountSettings Settings;
	Settings.TokenFraction = 0.4f;
	Settings.MinTokens = 1;
	Settings.MaxTokens = 3;

	TestEqual(TEXT("Empty group: no tokens"), EnemyCoordination::TokenCount(0, Settings), 0);
	TestEqual(TEXT("1 enemy: minimum 1"), EnemyCoordination::TokenCount(1, Settings), 1);
	TestEqual(TEXT("2 enemies: round(0.8) = 1"), EnemyCoordination::TokenCount(2, Settings), 1);
	TestEqual(TEXT("4 enemies: round(1.6) = 2"), EnemyCoordination::TokenCount(4, Settings), 2);
	TestEqual(TEXT("10 enemies: capped at 3"), EnemyCoordination::TokenCount(10, Settings), 3);

	Settings.MinTokens = 2;
	TestEqual(TEXT("Minimum never exceeds the group size"), EnemyCoordination::TokenCount(1, Settings), 1);

	const UEnemyCoordinationSettings* Project = GetDefault<UEnemyCoordinationSettings>();
	TestEqual(TEXT("Project defaults: 40 %, 1-3 -> 5 enemies get 2 tokens"), Project->GetTokenCount(5), 2);
	TestEqual(TEXT("Project default fraction"), Project->Tokens.TokenFraction, 0.4f);
	TestEqual(TEXT("Project default min"), Project->Tokens.MinTokens, 1);
	TestEqual(TEXT("Project default max"), Project->Tokens.MaxTokens, 3);
	return true;
}

#endif
