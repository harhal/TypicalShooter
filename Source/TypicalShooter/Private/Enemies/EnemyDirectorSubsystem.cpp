#include "Enemies/EnemyDirectorSubsystem.h"

#include "Enemies/Coordination/EnemyCoordinationSettings.h"
#include "Enemies/EnemyCharacter.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

void UEnemyDirectorSubsystem::Register(AEnemyCharacter* Enemy)
{
	if (Enemy)
	{
		Enemies.AddUnique(Enemy);
		Recompute();
	}
}

void UEnemyDirectorSubsystem::Unregister(AEnemyCharacter* Enemy)
{
	if (Enemies.Remove(Enemy) > 0)
	{
		Recompute();
	}
}

TArray<AEnemyCharacter*> UEnemyDirectorSubsystem::GetEnemies() const
{
	TArray<AEnemyCharacter*> Live;
	for (AEnemyCharacter* Enemy : Enemies)
	{
		if (IsValid(Enemy) && !Enemy->IsDead())
		{
			Live.Add(Enemy);
		}
	}
	return Live;
}

APawn* UEnemyDirectorSubsystem::FindTargetFor(const AActor* Enemy) const
{
	if (!Enemy)
	{
		return nullptr;
	}
	APawn* Nearest = nullptr;
	double NearestDistSq = TNumericLimits<double>::Max();
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APawn* Pawn = It->IsValid() ? (*It)->GetPawn() : nullptr;
		const double DistSq = Pawn ? FVector::DistSquared(Pawn->GetActorLocation(), Enemy->GetActorLocation()) : 0.0;
		if (IsValid(Pawn) && DistSq < NearestDistSq)
		{
			Nearest = Pawn;
			NearestDistSq = DistSq;
		}
	}
	return Nearest;
}

const FEnemyAssignment* UEnemyDirectorSubsystem::GetAssignment(const AEnemyCharacter* Enemy) const
{
	return Assignments.Find(Enemy);
}

bool UEnemyDirectorSubsystem::HasAttackToken(const AEnemyCharacter* Enemy) const
{
	const FGroup* Group = FindGroupOf(Enemy);
	return Group && Group->Coordinator.HasToken(Enemy->GetUniqueID());
}

void UEnemyDirectorSubsystem::ReleaseAttackToken(const AEnemyCharacter* Enemy)
{
	if (FGroup* Group = const_cast<FGroup*>(FindGroupOf(Enemy)))
	{
		Group->Coordinator.ReleaseToken(Enemy->GetUniqueID(), GetWorld()->GetTimeSeconds());
	}
}

TArray<APawn*> UEnemyDirectorSubsystem::GetGroupTargets() const
{
	TArray<APawn*> Targets;
	for (const FGroup& Group : Groups)
	{
		if (APawn* Target = Group.Target.Get())
		{
			Targets.Add(Target);
		}
	}
	return Targets;
}

TArray<AEnemyCharacter*> UEnemyDirectorSubsystem::GetGroupMembers(const APawn* Target) const
{
	TArray<AEnemyCharacter*> Members;
	for (const FGroup& Group : Groups)
	{
		if (Group.Target.Get() == Target)
		{
			for (const TWeakObjectPtr<AEnemyCharacter>& Member : Group.Members)
			{
				if (Member.IsValid())
				{
					Members.Add(Member.Get());
				}
			}
		}
	}
	return Members;
}

void UEnemyDirectorSubsystem::Tick(const float DeltaTime)
{
	SinceUpdate += DeltaTime;
	if (SinceUpdate >= GetDefault<UEnemyCoordinationSettings>()->UpdateInterval || AnyTargetChanged())
	{
		Recompute();
	}
}

TStatId UEnemyDirectorSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UEnemyDirectorSubsystem, STATGROUP_Tickables);
}

void UEnemyDirectorSubsystem::Recompute()
{
	SinceUpdate = 0.f;
	TArray<FGroup> Previous = MoveTemp(Groups);
	Groups.Reset();
	for (AEnemyCharacter* Enemy : GetEnemies())
	{
		if (APawn* Target = FindTargetFor(Enemy))
		{
			FGroup& Group = FindOrAddGroup(Target);
			if (Group.Members.IsEmpty())
			{
				// Keep the coordinator of a target that already had a group (tokens and flank view carry over).
				const int32 Old = Previous.IndexOfByPredicate([Target](const FGroup& G) { return G.Target.Get() == Target; });
				if (Old != INDEX_NONE)
				{
					Group.Coordinator = MoveTemp(Previous[Old].Coordinator);
				}
			}
			Group.Members.Add(Enemy);
		}
	}
	for (FGroup& Group : Groups)
	{
		UpdateGroup(Group);
	}
	RebuildAssignments();
}

bool UEnemyDirectorSubsystem::AnyTargetChanged() const
{
	for (AEnemyCharacter* Enemy : GetEnemies())
	{
		const FEnemyAssignment* Assignment = Assignments.Find(Enemy);
		const APawn* Current = Assignment ? Assignment->Target.Get() : nullptr;
		if (FindTargetFor(Enemy) != Current)
		{
			return true;
		}
	}
	return false;
}

UEnemyDirectorSubsystem::FGroup& UEnemyDirectorSubsystem::FindOrAddGroup(APawn* Target)
{
	if (FGroup* Existing = Groups.FindByPredicate([Target](const FGroup& G) { return G.Target.Get() == Target; }))
	{
		return *Existing;
	}
	FGroup& Group = Groups.AddDefaulted_GetRef();
	Group.Target = Target;
	return Group;
}

const UEnemyDirectorSubsystem::FGroup* UEnemyDirectorSubsystem::FindGroupOf(const AEnemyCharacter* Enemy) const
{
	return Enemy ? Groups.FindByPredicate([Enemy](const FGroup& G) { return G.Members.Contains(Enemy); }) : nullptr;
}

void UEnemyDirectorSubsystem::UpdateGroup(FGroup& Group)
{
	const APawn* Target = Group.Target.Get();
	TArray<FEnemyGroupMemberInput> Inputs;
	for (const TWeakObjectPtr<AEnemyCharacter>& Member : Group.Members)
	{
		FEnemyGroupMemberInput& Input = Inputs.AddDefaulted_GetRef();
		Input.Id = Member->GetUniqueID();
		Input.Location = Member->GetActorLocation();
		Input.bCanAttack = Member->CanShootAt(Target);
	}
	Group.Coordinator.Update(GetWorld()->GetTimeSeconds(), Target->GetActorLocation(), Target->GetViewRotation().Yaw, Inputs,
		*GetDefault<UEnemyCoordinationSettings>());
}

void UEnemyDirectorSubsystem::RebuildAssignments()
{
	Assignments.Reset();
	for (const FGroup& Group : Groups)
	{
		for (const TWeakObjectPtr<AEnemyCharacter>& Member : Group.Members)
		{
			if (const FEnemyGroupAssignment* Found = Group.Coordinator.Find(Member->GetUniqueID()))
			{
				FEnemyAssignment& Assignment = Assignments.Add(Member.Get());
				static_cast<FEnemyGroupAssignment&>(Assignment) = *Found;
				Assignment.Target = Group.Target;
			}
		}
	}
}
