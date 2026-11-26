// Tests for AEnemyWaveSpawner: waves after a delay, the next one when the current one is dead, looping, spawn points.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Enemies/EnemyDirectorSubsystem.h"
#include "Enemies/EnemyWaveSpawner.h"
#include "Tests/EnemiesTestFixture.h"

namespace EnemiesSpawnerTest
{
	FEnemyWave Wave(const int32 Count)
	{
		FEnemyWave Wave;
		Wave.Enemies.Add({ AEnemyCharacter::StaticClass(), Count });
		return Wave;
	}

	/** Spawner at (2000, 0) with the given wave sizes, 1 s before the first wave and between waves. */
	AEnemyWaveSpawner* SpawnSpawner(const ShootingTest::FWorldFixture& F, const TArray<int32>& WaveSizes, const TArray<AActor*>& Points = {})
	{
		AEnemyWaveSpawner* Spawner = F.World->SpawnActorDeferred<AEnemyWaveSpawner>(
			AEnemyWaveSpawner::StaticClass(), FTransform(FVector(2000.f, 0.f, 0.f)));
		Spawner->Waves.Reset();
		for (const int32 Size : WaveSizes)
		{
			Spawner->Waves.Add(Wave(Size));
		}
		Spawner->FirstWaveDelay = 1.f;
		Spawner->WaveDelay = 1.f;
		Spawner->SpawnRadius = 400.f;
		Spawner->SpawnPoints = Points;
		Spawner->FinishSpawning(FTransform(FVector(2000.f, 0.f, 0.f)));
		return Spawner;
	}

	TArray<AEnemyCharacter*> Enemies(const ShootingTest::FWorldFixture& F)
	{
		return F.World->GetSubsystem<UEnemyDirectorSubsystem>()->GetEnemies();
	}

	void KillAll(const ShootingTest::FWorldFixture& F)
	{
		for (AEnemyCharacter* Enemy : Enemies(F))
		{
			EnemiesTest::Hit(Enemy, 10000.f);
		}
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesSpawnerFirstWaveTest, "TypicalShooter.Enemies.Spawner.FirstWaveAfterDelay", ShootingTest::Flags)
bool FEnemiesSpawnerFirstWaveTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesSpawnerTest;
	const ShootingTest::FWorldFixture F(true);
	EnemiesTest::SpawnFloor(F);
	const AEnemyWaveSpawner* Spawner = SpawnSpawner(F, { 2, 3 });

	F.Tick(0.5f);
	TestEqual(TEXT("Nothing before the first wave delay"), Enemies(F).Num(), 0);
	F.Tick(0.7f);
	TestEqual(TEXT("First wave spawned"), Enemies(F).Num(), 2);
	TestEqual(TEXT("Wave index"), Spawner->GetWaveIndex(), 0);
	TestEqual(TEXT("Alive count"), Spawner->GetAliveCount(), 2);
	for (const AEnemyCharacter* Enemy : Enemies(F))
	{
		TestTrue(TEXT("Spawned within the radius around the spawner"), FVector::Dist2D(Enemy->GetActorLocation(), Spawner->GetActorLocation()) <= 400.0 + 1.0);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesSpawnerNextWaveTest, "TypicalShooter.Enemies.Spawner.NextWaveWhenAllDead", ShootingTest::Flags)
bool FEnemiesSpawnerNextWaveTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesSpawnerTest;
	const ShootingTest::FWorldFixture F(true);
	EnemiesTest::SpawnFloor(F);
	const AEnemyWaveSpawner* Spawner = SpawnSpawner(F, { 2, 3 });
	F.Tick(1.2f);
	if (Enemies(F).Num() != 2)
	{
		AddError(TEXT("The first wave did not spawn"));
		return false;
	}

	EnemiesTest::Hit(Enemies(F)[0], 10000.f);
	F.Tick(2.f);
	TestEqual(TEXT("No new wave while one enemy lives"), Spawner->GetWaveIndex(), 0);
	TestEqual(TEXT("One left"), Spawner->GetAliveCount(), 1);

	KillAll(F);
	F.Tick(0.5f);
	TestEqual(TEXT("Waits the wave delay"), Enemies(F).Num(), 0);
	F.Tick(0.7f);
	TestEqual(TEXT("Second wave"), Spawner->GetWaveIndex(), 1);
	TestEqual(TEXT("Second wave size"), Enemies(F).Num(), 3);

	KillAll(F);
	F.Tick(1.2f);
	TestEqual(TEXT("After the last wave the list repeats"), Spawner->GetWaveIndex(), 0);
	TestEqual(TEXT("First wave size again"), Enemies(F).Num(), 2);
	TestEqual(TEXT("Waves started in total"), Spawner->GetWavesStarted(), 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesSpawnerDisabledTest, "TypicalShooter.Enemies.Spawner.DisabledByConsoleVariable", ShootingTest::Flags)
bool FEnemiesSpawnerDisabledTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesSpawnerTest;
	const ShootingTest::FWorldFixture F(true);
	EnemiesTest::SpawnFloor(F);
	AEnemyWaveSpawner::SetWavesEnabled(false);
	SpawnSpawner(F, { 2 });
	AEnemyWaveSpawner::SetWavesEnabled(true);
	F.Tick(2.f);
	TestEqual(TEXT("A spawner that began play with waves disabled spawns nothing"), Enemies(F).Num(), 0);
	TestTrue(TEXT("Enabled again"), AEnemyWaveSpawner::AreWavesEnabled());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemiesSpawnerPointsTest, "TypicalShooter.Enemies.Spawner.UsesSpawnPoints", ShootingTest::Flags)
bool FEnemiesSpawnerPointsTest::RunTest(const FString& Parameters)
{
	using namespace EnemiesSpawnerTest;
	const ShootingTest::FWorldFixture F(true);
	EnemiesTest::SpawnFloor(F);
	AActor* A = F.World->SpawnActor<AActor>();
	AActor* B = F.World->SpawnActor<AActor>();
	A->SetRootComponent(NewObject<USceneComponent>(A));
	B->SetRootComponent(NewObject<USceneComponent>(B));
	A->SetActorLocation(FVector(-1500.f, 1500.f, 0.f));
	B->SetActorLocation(FVector(-1500.f, -1500.f, 0.f));
	SpawnSpawner(F, { 2 }, { A, B });
	F.Tick(1.2f);

	for (const AEnemyCharacter* Enemy : Enemies(F))
	{
		const double ToPoint = FMath::Min(FVector::Dist2D(Enemy->GetActorLocation(), A->GetActorLocation()),
			FVector::Dist2D(Enemy->GetActorLocation(), B->GetActorLocation()));
		TestTrue(TEXT("Spawned at one of the spawn points"), ToPoint < 50.0);
	}
	TestEqual(TEXT("Both spawned"), Enemies(F).Num(), 2);
	return true;
}

#endif
