// Runs the generated enemy StateTree in a bare world: the active child of Combat follows the role the director gives the
// enemy (Free without coordination), and a role change does not restart Combat (the attack token is kept).

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Enemies/BuildEnemyStateTreeCommandlet.h"
#include "Enemies/EnemyAIController.h"
#include "Enemies/EnemyDirectorSubsystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "StateTree.h"
#include "Tests/EnemiesTestFixture.h"

namespace EnemiesBehaviorRoleStatesTest
{
	bool InState(const AEnemyAIController& Controller, const TCHAR* State)
	{
		return Controller.GetActiveStateNames().Contains(FName(State));
	}

	/** Ticks until the controller is in State (at most Seconds); true if it got there. */
	bool TickUntilState(const ShootingTest::FWorldFixture& F, const AEnemyAIController& Controller, const TCHAR* State, const float Seconds)
	{
		for (float Elapsed = 0.f; Elapsed < Seconds; Elapsed += 1.f / 60.f)
		{
			if (InState(Controller, State))
			{
				return true;
			}
			F.Tick(1.f / 60.f);
		}
		return InState(Controller, State);
	}

	FString StatesOf(const AEnemyAIController& Controller)
	{
		return FString::JoinBy(Controller.GetActiveStateNames(), TEXT("/"), [](const FName Name) { return Name.ToString(); });
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesBehaviorRoleStatesTest, "TypicalShooter.Enemies.Behavior.RoleStates", ShootingTest::Flags)
bool FEnemiesBehaviorRoleStatesTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesBehaviorRoleStatesTest;
	UStateTree* Tree = NewObject<UStateTree>(GetTransientPackage());
	if (!TestTrue(TEXT("The enemy tree compiles"), UBuildEnemyStateTreeCommandlet::BuildTree(*Tree)))
	{
		return false;
	}

	const ShootingTest::FWorldFixture F(true);
	EnemiesTest::AddNavigation(F);
	EnemiesTest::SpawnFloor(F);
	AEnemyCharacter* Enemy = EnemiesTest::SpawnEnemy(F);
	AEnemyAIController* Controller = Cast<AEnemyAIController>(Enemy->GetController());
	if (!TestNotNull(TEXT("AI controller"), Controller))
	{
		return false;
	}
	Controller->Behavior = Tree;
	Controller->UnPossess();
	Controller->Possess(Enemy);

	TestTrue(FString::Printf(TEXT("No player: Free (%s)"), *StatesOf(*Controller)), TickUntilState(F, *Controller, TEXT("Free"), 0.5f));

	APlayerController* PlayerController = F.World->SpawnActor<APlayerController>();
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	PlayerController->Possess(F.World->SpawnActor<ACharacter>(FVector(1200.f, 0.f, 100.f), FRotator::ZeroRotator, Params));
	TestTrue(FString::Printf(TEXT("A lone enemy presses (%s)"), *StatesOf(*Controller)), TickUntilState(F, *Controller, TEXT("Pressure"), 1.f));
	TestTrue(TEXT("Combat stays the parent"), InState(*Controller, TEXT("Combat")));
	TestTrue(TEXT("It holds the attack token"), Controller->HasAttackToken());

	Enemy->AllowedRoles = {EEnemyRole::Cover};
	TestTrue(FString::Printf(TEXT("Role changed: Cover (%s)"), *StatesOf(*Controller)), TickUntilState(F, *Controller, TEXT("Cover"), 1.5f));
	F.Tick(0.1f);
	TestTrue(TEXT("Switching role states does not restart Combat: the token is kept"), Controller->HasAttackToken());
	return true;
}

#endif
