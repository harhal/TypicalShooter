#include "Enemies/Coordination/EnemyGroupCoordinator.h"

#include "Enemies/Coordination/EnemyCoordinationSettings.h"
#include "Enemies/Coordination/EnemyRoleAssignment.h"
#include "Enemies/Coordination/EnemySectors.h"

void FEnemyGroupCoordinator::Update(const float Now, const FVector& TargetLocation, const float TargetViewYaw,
	const TArray<FEnemyGroupMemberInput>& Members, const UEnemyCoordinationSettings& Settings)
{
	UpdateRoles(Members, Settings);
	UpdateSectors(TargetLocation, TargetViewYaw, Members, Settings);
	UpdateTokens(Now, TargetLocation, Members, Settings);
}

void FEnemyGroupCoordinator::UpdateRoles(const TArray<FEnemyGroupMemberInput>& Members, const UEnemyCoordinationSettings& Settings)
{
	TArray<EnemyRoles::FCandidate> Candidates;
	for (const FEnemyGroupMemberInput& Member : Members)
	{
		EnemyRoles::FCandidate& Candidate = Candidates.AddDefaulted_GetRef();
		Candidate.Allowed = Member.AllowedRoles;
		if (const FEnemyGroupAssignment* Previous = Assignments.Find(Member.Id))
		{
			Candidate.Current = Previous->Role;
		}
	}
	const TArray<EEnemyRole> Roles = EnemyRoles::Assign(Candidates, Settings.RoleWeights);

	TMap<int32, FEnemyGroupAssignment> Updated;
	for (int32 Index = 0; Index < Members.Num(); ++Index)
	{
		FEnemyGroupAssignment& Assignment = Updated.Add(Members[Index].Id);
		Assignment.Role = Roles[Index];
	}
	Assignments = MoveTemp(Updated);
}

void FEnemyGroupCoordinator::UpdateSectors(const FVector& TargetLocation, const float TargetViewYaw,
	const TArray<FEnemyGroupMemberInput>& Members, const UEnemyCoordinationSettings& Settings)
{
	if (!FlankViewYaw.IsSet() || EnemySectors::ShouldReaimFlank(FlankViewYaw.GetValue(), TargetViewYaw, Settings.Sectors.FlankReaimDegrees))
	{
		FlankViewYaw = TargetViewYaw;
	}
	FEnemySectorQuery Query;
	Query.ViewYaw = FlankViewYaw.GetValue();
	Query.FlankViewHalfAngle = Settings.Sectors.FlankViewHalfAngle;
	for (const FEnemyGroupMemberInput& Member : Members)
	{
		Query.Members.Add({Assignments[Member.Id].Role, EnemySectors::YawAround(TargetLocation, Member.Location)});
	}
	const TArray<float> Yaws = EnemySectors::Assign(Query);
	const float HalfWidth = EnemySectors::SectorHalfWidth(Members.Num());
	for (int32 Index = 0; Index < Members.Num(); ++Index)
	{
		FEnemyGroupAssignment& Assignment = Assignments[Members[Index].Id];
		Assignment.SectorYaw = Yaws[Index];
		Assignment.SectorHalfWidth = HalfWidth;
	}
}

void FEnemyGroupCoordinator::UpdateTokens(const float Now, const FVector& TargetLocation, const TArray<FEnemyGroupMemberInput>& Members,
	const UEnemyCoordinationSettings& Settings)
{
	TArray<FEnemyTokenCandidate> Candidates;
	for (const FEnemyGroupMemberInput& Member : Members)
	{
		Candidates.Add({Member.Id, Assignments[Member.Id].Role, Member.bCanAttack, float(FVector::Dist(TargetLocation, Member.Location))});
	}
	Tokens.Update(Now, Candidates, Settings.GetTokenCount(Members.Num()), Settings.TokenRotation);
}
