// Spawns enemies in waves (server): the first wave after FirstWaveDelay, each next one WaveDelay after the whole current
// wave died; after the last wave the list repeats.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Math/RandomStream.h"
#include "EnemyWaveSpawner.generated.h"

class AEnemyCharacter;

USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FEnemyWaveEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	TSubclassOf<AEnemyCharacter> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave", meta = (ClampMin = 1))
	int32 Count = 1;
};

USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FEnemyWave
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	TArray<FEnemyWaveEntry> Enemies;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEnemyWaveStarted, int32, WaveIndex);

UCLASS()
class TYPICALSHOOTER_API AEnemyWaveSpawner : public AActor
{
	GENERATED_BODY()

public:
	AEnemyWaveSpawner();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves")
	TArray<FEnemyWave> Waves;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves", meta = (ClampMin = 0))
	float FirstWaveDelay = 2.f;

	/** Pause between the death of the last enemy of a wave and the next wave. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves", meta = (ClampMin = 0))
	float WaveDelay = 3.f;

	/** Enemies appear at these actors in turn; empty = random points within SpawnRadius around the spawner. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Spawn")
	TArray<TObjectPtr<AActor>> SpawnPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = 0))
	float SpawnRadius = 500.f;

	UPROPERTY(BlueprintAssignable, Category = "Waves")
	FEnemyWaveStarted OnWaveStarted;

	/** Index in Waves of the current wave; -1 before the first one. */
	UFUNCTION(BlueprintPure, Category = "Waves")
	int32 GetWaveIndex() const { return WaveIndex; }

	UFUNCTION(BlueprintPure, Category = "Waves")
	int32 GetWavesStarted() const { return WavesStarted; }

	UFUNCTION(BlueprintPure, Category = "Waves")
	int32 GetAliveCount() const { return Alive.Num(); }

	/** Console variable TypicalShooter.Enemies.Waves: 0 = spawners that begin play start no waves (tests, debugging). */
	static bool AreWavesEnabled();
	static void SetWavesEnabled(bool bEnabled);

protected:
	virtual void BeginPlay() override;

private:
	void ScheduleNextWave(float Delay);
	void StartNextWave();
	void SpawnEnemy(TSubclassOf<AEnemyCharacter> EnemyClass);
	FVector NextSpawnLocation();
	void OnEnemyGone(AEnemyCharacter* Enemy);

	/** An enemy destroyed without dying (level unload, editor) also leaves the wave. */
	UFUNCTION()
	void OnEnemyEndPlay(AActor* Actor, EEndPlayReason::Type Reason);

	UPROPERTY(Transient)
	TArray<TObjectPtr<AEnemyCharacter>> Alive;

	int32 WaveIndex = -1;
	int32 WavesStarted = 0;
	int32 NextSpawnPoint = 0;
	FTimerHandle WaveTimer;
	FRandomStream Random;
};
