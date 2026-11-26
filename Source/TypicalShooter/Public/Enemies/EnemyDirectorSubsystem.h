// Knows every live enemy of the world (server) and coordinates them: enemies sharing a target form a group whose roles,
// sectors around the target and attack tokens are recomputed on register / death, when an enemy's target changes and
// every UpdateInterval (UEnemyCoordinationSettings).

#pragma once

#include "CoreMinimal.h"
#include "Enemies/Coordination/EnemyGroupCoordinator.h"
#include "Subsystems/WorldSubsystem.h"
#include "EnemyDirectorSubsystem.generated.h"

class AEnemyCharacter;

/** What the director decided for one enemy. */
struct FEnemyAssignment : FEnemyGroupAssignment
{
	TWeakObjectPtr<APawn> Target;
};

UCLASS()
class TYPICALSHOOTER_API UEnemyDirectorSubsystem : public UTickableWorldSubsystem
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

	/** Role / sector / target of a registered enemy; null if it has no target (or is not registered). */
	const FEnemyAssignment* GetAssignment(const AEnemyCharacter* Enemy) const;

	/** The enemy may attack now. */
	bool HasAttackToken(const AEnemyCharacter* Enemy) const;

	/** The enemy gives its token back now (left combat); its cooldown starts. */
	void ReleaseAttackToken(const AEnemyCharacter* Enemy);

	TArray<APawn*> GetGroupTargets() const;
	TArray<AEnemyCharacter*> GetGroupMembers(const APawn* Target) const;

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

private:
	struct FGroup
	{
		TWeakObjectPtr<APawn> Target;
		TArray<TWeakObjectPtr<AEnemyCharacter>> Members;
		FEnemyGroupCoordinator Coordinator;
	};

	void Recompute();
	bool AnyTargetChanged() const;
	FGroup& FindOrAddGroup(APawn* Target);
	const FGroup* FindGroupOf(const AEnemyCharacter* Enemy) const;
	void UpdateGroup(FGroup& Group);
	void RebuildAssignments();

	UPROPERTY(Transient)
	TArray<TObjectPtr<AEnemyCharacter>> Enemies;

	TArray<FGroup> Groups;
	TMap<TWeakObjectPtr<const AEnemyCharacter>, FEnemyAssignment> Assignments;
	float SinceUpdate = 0.f;
};
