// Tests for the coordination debug drawing: nothing is drawn while TypicalShooter.Enemies.Debug.Coordination is 0,
// the groups are drawn while it is 1.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS && ENABLE_DRAW_DEBUG

#include "Components/LineBatchComponent.h"
#include "Enemies/EnemyDirectorSubsystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "Tests/EnemiesTestFixture.h"

namespace EnemiesCoordinationDebugTest
{
	/** Lines the director draws in one tick. */
	int32 LinesDrawnByOneTick(const ShootingTest::FWorldFixture& F)
	{
		ULineBatchComponent* Lines = F.World->GetLineBatcher(UWorld::ELineBatcherType::World);
		Lines->Flush();
		F.World->GetSubsystem<UEnemyDirectorSubsystem>()->Tick(0.01f);
		return Lines->BatchedLines.Num();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesCoordinationDebugTest, "TypicalShooter.Enemies.Coordination.DebugDraw", ShootingTest::Flags)
bool FEnemiesCoordinationDebugTest::RunTest(const FString& Parameters)
{
	IConsoleVariable* Debug = IConsoleManager::Get().FindConsoleVariable(TEXT("TypicalShooter.Enemies.Debug.Coordination"));
	if (!TestNotNull(TEXT("The console variable exists"), Debug))
	{
		return false;
	}
	const int32 Previous = Debug->GetInt();

	const ShootingTest::FWorldFixture F(true);
	EnemiesTest::SpawnFloor(F);
	APlayerController* Controller = F.World->SpawnActor<APlayerController>();
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Controller->Possess(F.World->SpawnActor<ACharacter>(FVector(0.f, 0.f, 100.f), FRotator::ZeroRotator, Params));
	EnemiesTest::SpawnEnemy(F, FVector(1000.f, 0.f, 0.f));
	EnemiesTest::SpawnEnemy(F, FVector(-1000.f, 0.f, 0.f));

	Debug->Set(0, ECVF_SetByCode);
	TestEqual(TEXT("Off: nothing drawn"), EnemiesCoordinationDebugTest::LinesDrawnByOneTick(F), 0);

	Debug->Set(1, ECVF_SetByCode);
	const int32 Drawn = EnemiesCoordinationDebugTest::LinesDrawnByOneTick(F);
	TestTrue(FString::Printf(TEXT("On: sectors, view cone and stand lines drawn (%d lines)"), Drawn), Drawn >= 2);

	Debug->Set(Previous, ECVF_SetByCode);
	return true;
}

#endif
