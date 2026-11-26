#include "Enemies/EnemyWaveSpawner.h"

#include "Components/CapsuleComponent.h"
#include "Enemies/EnemyCharacter.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "TimerManager.h"
#include "TypicalShooter.h"

namespace EnemyWaveSpawnerCVars
{
	static TAutoConsoleVariable<bool> CVarWavesEnabled(TEXT("TypicalShooter.Enemies.Waves"), true,
		TEXT("Enemy wave spawners start their waves when they begin play (0 = no waves; for tests and debugging)."));
}

bool AEnemyWaveSpawner::AreWavesEnabled()
{
	return EnemyWaveSpawnerCVars::CVarWavesEnabled.GetValueOnGameThread();
}

void AEnemyWaveSpawner::SetWavesEnabled(const bool bEnabled)
{
	EnemyWaveSpawnerCVars::CVarWavesEnabled->Set(bEnabled, ECVF_SetByCode);
}

AEnemyWaveSpawner::AEnemyWaveSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Random.GenerateNewSeed();
	for (const int32 Count : { 2, 3, 4 })
	{
		FEnemyWave Wave;
		Wave.Enemies.Add({ AEnemyCharacter::StaticClass(), Count });
		Waves.Add(Wave);
	}
}

void AEnemyWaveSpawner::BeginPlay()
{
	Super::BeginPlay();
	if (!AreWavesEnabled())
	{
		UE_LOG(LogTypicalShooter, Log, TEXT("%s: waves disabled (TypicalShooter.Enemies.Waves 0)"), *GetName());
	}
	else if (HasAuthority() && Waves.Num() > 0)
	{
		ScheduleNextWave(FirstWaveDelay);
	}
}

void AEnemyWaveSpawner::ScheduleNextWave(const float Delay)
{
	if (Delay <= 0.f)
	{
		StartNextWave();
		return;
	}
	GetWorldTimerManager().SetTimer(WaveTimer, this, &AEnemyWaveSpawner::StartNextWave, Delay);
}

void AEnemyWaveSpawner::StartNextWave()
{
	WaveIndex = (WaveIndex + 1) % Waves.Num();
	++WavesStarted;
	for (const FEnemyWaveEntry& Entry : Waves[WaveIndex].Enemies)
	{
		for (int32 Index = 0; Index < Entry.Count; ++Index)
		{
			SpawnEnemy(Entry.EnemyClass);
		}
	}
	UE_LOG(LogTypicalShooter, Log, TEXT("%s: wave %d started, %d enemies"), *GetName(), WaveIndex + 1, Alive.Num());
	OnWaveStarted.Broadcast(WaveIndex);
	if (Alive.IsEmpty())
	{
		ScheduleNextWave(WaveDelay); // nothing could spawn: do not stall
	}
}

void AEnemyWaveSpawner::SpawnEnemy(const TSubclassOf<AEnemyCharacter> EnemyClass)
{
	if (!EnemyClass)
	{
		return;
	}
	const float HalfHeight = EnemyClass.GetDefaultObject()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector Location = NextSpawnLocation() + FVector(0.f, 0.f, HalfHeight + 5.f);
	const FRotator Rotation(0.f, Random.FRandRange(-180.f, 180.f), 0.f);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	AEnemyCharacter* Enemy = GetWorld()->SpawnActor<AEnemyCharacter>(EnemyClass, Location, Rotation, Params);
	if (Enemy && !Enemy->IsDead())
	{
		Alive.Add(Enemy);
		Enemy->GetDiedDelegate().AddUObject(this, &AEnemyWaveSpawner::OnEnemyGone);
		Enemy->OnEndPlay.AddDynamic(this, &AEnemyWaveSpawner::OnEnemyEndPlay);
	}
}

FVector AEnemyWaveSpawner::NextSpawnLocation()
{
	for (int32 Tries = 0; Tries < SpawnPoints.Num(); ++Tries)
	{
		const AActor* Point = SpawnPoints[NextSpawnPoint++ % SpawnPoints.Num()];
		if (Point)
		{
			return Point->GetActorLocation();
		}
	}
	const FVector2D Offset = FVector2D(Random.GetUnitVector()).GetSafeNormal() * SpawnRadius * FMath::Sqrt(Random.FRand());
	return GetActorLocation() + FVector(Offset, 0.f);
}

void AEnemyWaveSpawner::OnEnemyEndPlay(AActor* Actor, const EEndPlayReason::Type Reason)
{
	OnEnemyGone(Cast<AEnemyCharacter>(Actor));
}

void AEnemyWaveSpawner::OnEnemyGone(AEnemyCharacter* Enemy)
{
	if (Alive.Remove(Enemy) > 0 && Alive.IsEmpty() && HasActorBegunPlay())
	{
		ScheduleNextWave(WaveDelay);
	}
}
