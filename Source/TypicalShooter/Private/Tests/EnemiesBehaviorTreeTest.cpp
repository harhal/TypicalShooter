// Tests that the enemy behaviour StateTree built from code compiles (the asset ST_Enemy is generated the same way).

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Enemies/BuildEnemyStateTreeCommandlet.h"
#include "StateTree.h"
#include "Tests/ShootingTestFixture.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesBehaviorTreeCompilesTest, "TypicalShooter.Enemies.Behavior.TreeCompiles", ShootingTest::Flags)
bool FEnemiesBehaviorTreeCompilesTest::RunTest(const FString& Parameters)
{
	UStateTree* Tree = NewObject<UStateTree>(GetTransientPackage());
	TestTrue(TEXT("The enemy tree compiles"), UBuildEnemyStateTreeCommandlet::BuildTree(*Tree));
	TestTrue(TEXT("Compiled tree is ready to run"), Tree->IsReadyToRun());
	TestEqual(TEXT("Root + Combat + Pressure / Flank / Cover / Free states"), Tree->GetStates().Num(), 6);
	return true;
}

#endif
