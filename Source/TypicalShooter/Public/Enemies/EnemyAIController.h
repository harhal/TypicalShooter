// AI controller of AEnemyCharacter. Runs the behaviour StateTree (UStateTreeAIComponent); the StateTree tasks call the
// small steps below every tick: track the target, keep the distance band, shoot when visible.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Math/RandomStream.h"
#include "EnemyAIController.generated.h"

class AEnemyCharacter;
class UStateTree;
class UStateTreeAIComponent;

/** Distance band and movement tuning of the "keep distance" task. */
USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FEnemyStandSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance", meta = (ClampMin = 0))
	float MinDistance = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance", meta = (ClampMin = 0))
	float MaxDistance = 1600.f;

	/** Random turn of the stand point around the target, degrees. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance", meta = (ClampMin = 0, ClampMax = 180))
	float JitterDegrees = 20.f;

	/** Random turn around the target when coming closer without line of sight, degrees. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance", meta = (ClampMin = 0, ClampMax = 180))
	float FlankDegrees = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = 0))
	float AcceptanceRadius = 100.f;

	/** While moving, the stand point is chosen again after this many seconds (the target moves too). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = 0.1))
	float RepathInterval = 1.5f;
};

UCLASS()
class TYPICALSHOOTER_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyAIController();

	/** Behaviour StateTree (AI component schema); started when a pawn is possessed. None = no logic (tests). */
	UPROPERTY(EditDefaultsOnly, Category = "Enemy AI")
	TObjectPtr<UStateTree> Behavior;

	UFUNCTION(BlueprintPure, Category = "Enemy AI")
	AActor* GetTarget() const { return Target; }

	/** Target = the director's choice (nearest player pawn); look at it. */
	UFUNCTION(BlueprintCallable, Category = "Enemy AI")
	void UpdateTarget();

	/** Fire while the target is in range and visible from the muzzle. */
	UFUNCTION(BlueprintCallable, Category = "Enemy AI")
	void UpdateFire();

	/**
	 * Keep within the distance band of the target: when idle (or RepathInterval elapsed) choose a stand point and move there.
	 * Returns true when a new move was started.
	 */
	UFUNCTION(BlueprintCallable, Category = "Enemy AI")
	bool UpdatePosition(const FEnemyStandSettings& Settings);

	/** Last chosen stand point. */
	UFUNCTION(BlueprintPure, Category = "Enemy AI")
	FVector GetStandPoint() const { return StandPoint; }

	/** Stop firing, moving and looking at the target. */
	UFUNCTION(BlueprintCallable, Category = "Enemy AI")
	void StopCombat();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

private:
	AEnemyCharacter* GetEnemy() const;
	void MoveToStandPoint(const FEnemyStandSettings& Settings);
	/** Snaps Point onto the navmesh; false if it is off the navmesh (outside the arena, inside a wall). */
	bool ProjectToNavigation(FVector& Point) const;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStateTreeAIComponent> StateTreeAI;

	UPROPERTY(Transient)
	TObjectPtr<AActor> Target;

	FVector StandPoint = FVector::ZeroVector;
	double LastStandChoiceTime = -1.0;
	FRandomStream Random;
};
