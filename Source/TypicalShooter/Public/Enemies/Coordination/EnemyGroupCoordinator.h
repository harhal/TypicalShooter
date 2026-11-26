// Coordination of one enemy group (the enemies sharing a target): roles, sectors and attack tokens, recomputed from
// plain inputs. No world access; the director feeds it.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/Coordination/EnemyRole.h"
#include "Enemies/Coordination/EnemyTokenPool.h"

class UEnemyCoordinationSettings;

struct FEnemyGroupMemberInput
{
	int32 Id = INDEX_NONE;
	/** EnemyRoles::Mask bits; 0 = any role. */
	uint8 AllowedRoles = 0;
	FVector Location = FVector::ZeroVector;
	/** The enemy could attack the target right now (line of sight and range). */
	bool bCanAttack = false;
};

struct FEnemyGroupAssignment
{
	EEnemyRole Role = EEnemyRole::Pressure;
	float SectorYaw = 0.f;
	float SectorHalfWidth = 180.f;
};

class TYPICALSHOOTER_API FEnemyGroupCoordinator
{
public:
	/** Recomputes the whole group at time Now; members missing from Members leave it. */
	void Update(float Now, const FVector& TargetLocation, float TargetViewYaw, const TArray<FEnemyGroupMemberInput>& Members,
		const UEnemyCoordinationSettings& Settings);

	/** Null if Id is not in the group. */
	const FEnemyGroupAssignment* Find(int32 Id) const { return Assignments.Find(Id); }

	bool HasToken(int32 Id) const { return Tokens.HasToken(Id); }
	void ReleaseToken(int32 Id, float Now) { Tokens.Release(Id, Now); }

	int32 Num() const { return Assignments.Num(); }

private:
	void UpdateRoles(const TArray<FEnemyGroupMemberInput>& Members, const UEnemyCoordinationSettings& Settings);
	void UpdateSectors(const FVector& TargetLocation, float TargetViewYaw, const TArray<FEnemyGroupMemberInput>& Members,
		const UEnemyCoordinationSettings& Settings);
	void UpdateTokens(float Now, const FVector& TargetLocation, const TArray<FEnemyGroupMemberInput>& Members,
		const UEnemyCoordinationSettings& Settings);

	TMap<int32, FEnemyGroupAssignment> Assignments;
	FEnemyTokenPool Tokens;
	/** Target view yaw the flank sectors are based on (hysteresis). */
	TOptional<float> FlankViewYaw;
};
