// Attack tokens of one enemy group over time: who may attack right now. Pure logic, the caller passes the time and the
// group's candidates; members are identified by an id the caller chooses.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/Coordination/EnemyRole.h"
#include "EnemyTokenPool.generated.h"

USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FEnemyTokenRotationSettings
{
	GENERATED_BODY()

	/** A holder gives the token back after this long if someone else is waiting. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tokens", meta = (ClampMin = "0", Units = "s"))
	float TokenHoldTime = 3.f;

	/** A holder that has not seen the target for this long gives the token back. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tokens", meta = (ClampMin = "0", Units = "s"))
	float LostSightGrace = 0.5f;

	/** After giving a token back an enemy yields to others for this long. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tokens", meta = (ClampMin = "0", Units = "s"))
	float TokenCooldown = 1.5f;

	/** An enemy outside the target's view (attacking from the side / behind) holds a token for this share of TokenHoldTime. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tokens", meta = (ClampMin = "0", ClampMax = "1"))
	float OutOfViewHoldScale = 0.5f;

	/** ... and its cooldown is this many times TokenCooldown, so it gets tokens less often. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tokens", meta = (ClampMin = "1"))
	float OutOfViewCooldownScale = 3.f;
};

struct FEnemyTokenCandidate
{
	int32 Id = INDEX_NONE;
	EEnemyRole Role = EEnemyRole::Pressure;
	bool bSeesTarget = false;
	float DistanceToTarget = 0.f;
	/** The enemy is within the target's view cone. */
	bool bInTargetView = true;
};

/**
 * Tokens go only to enemies that see the target, ordered by role (Pressure, Flank, Cover), then by how long they have been
 * waiting, then by distance. A holder keeps its token up to TokenHoldTime (longer if nobody else could take it), loses it
 * after LostSightGrace without sight, and then yields for TokenCooldown unless nobody else wants a token.
 */
class TYPICALSHOOTER_API FEnemyTokenPool
{
public:
	/** Re-evaluates the group at time Now: members missing from Candidates leave (their tokens free up). */
	void Update(float Now, const TArray<FEnemyTokenCandidate>& Candidates, int32 TokenCount, const FEnemyTokenRotationSettings& Settings);

	/** The member gives its token back now (left combat, died); the cooldown starts. */
	void Release(int32 Id, float Now);

	bool HasToken(int32 Id) const;
	TArray<int32> GetHolders() const;

private:
	struct FMemberState
	{
		bool bHolding = false;
		float HoldStart = 0.f;
		float WaitingSince = 0.f;
		float ReleasedAt = -UE_BIG_NUMBER;
		TOptional<float> LostSightSince;
		FEnemyTokenCandidate Last;
	};

	void SyncMembers(float Now, const TArray<FEnemyTokenCandidate>& Candidates);
	void ReleaseExpired(float Now, const FEnemyTokenRotationSettings& Settings);
	void ReleaseOverflow(float Now, int32 TokenCount);
	void Grant(float Now, int32 TokenCount, const FEnemyTokenRotationSettings& Settings);
	static void GiveBack(FMemberState& State, float Now);
	static bool IsBefore(const FMemberState& A, const FMemberState& B);
	static bool IsClaimant(const FMemberState& State);
	bool HasOtherClaimant(int32 Except) const;
	int32 CountHolders() const;

	TMap<int32, FMemberState> Members;
};