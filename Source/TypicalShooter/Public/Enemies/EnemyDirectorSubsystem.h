// Knows every live enemy of the world (server). The place for group coordination later (who fires, who stands where);
// for now it registers enemies and picks their target.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "EnemyDirectorSubsystem.generated.h"

class AEnemyCharacter;

UCLASS()
class TYPICALSHOOTER_API UEnemyDirectorSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Called by the enemy on BeginPlay (server). */
	void Register(AEnemyCharacter* Enemy);

	/** Called by the enemy on death / EndPlay. */
	void Unregister(AEnemyCharacter* Enemy);

	/** Live enemies. */
	TArray<AEnemyCharacter*> GetEnemies() const;

	/** The nearest pawn controlled by a player; null if none. */
	UFUNCTION(BlueprintPure, Category = "Enemies")
	APawn* FindTargetFor(const AActor* Enemy) const;

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<AEnemyCharacter>> Enemies;
};
