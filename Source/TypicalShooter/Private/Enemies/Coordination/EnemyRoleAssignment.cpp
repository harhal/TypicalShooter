#include "Enemies/Coordination/EnemyRoleAssignment.h"

#include "Algo/StableSort.h"

namespace EnemyRoleAssignment
{
	using namespace EnemyRoles;

	EEnemyRole RoleAt(const int32 Index) { return static_cast<EEnemyRole>(Index); }
	int32 IndexOf(const EEnemyRole Role) { return static_cast<int32>(Role); }

	uint8 AllowedOf(const FCandidate& Candidate) { return Candidate.Allowed != 0 ? Candidate.Allowed : AllRoles; }
	bool Allows(const uint8 Allowed, const int32 Role) { return (Allowed & Mask(RoleAt(Role))) != 0; }

	/** Working state of one Assign call. */
	struct FState
	{
		const TArray<FCandidate>& Candidates;
		FQuotas Quotas;
		FQuotas Counts;
		TArray<int32> Roles; // INDEX_NONE = not assigned yet

		int32 Room(const int32 Role) const { return Quotas[Role] - Counts[Role]; }

		void Set(const int32 Member, const int32 Role)
		{
			if (Roles[Member] != INDEX_NONE)
			{
				--Counts[Roles[Member]];
			}
			Roles[Member] = Role;
			++Counts[Role];
		}
	};

	void KeepCurrentRoles(FState& State)
	{
		for (int32 Member = 0; Member < State.Candidates.Num(); ++Member)
		{
			const FCandidate& Candidate = State.Candidates[Member];
			if (Candidate.Current.IsSet())
			{
				const int32 Role = IndexOf(Candidate.Current.GetValue());
				if (Allows(AllowedOf(Candidate), Role) && State.Room(Role) > 0)
				{
					State.Set(Member, Role);
				}
			}
		}
	}

	/** Allowed role with the most room left (earlier role on ties); INDEX_NONE if all allowed roles are full. */
	int32 FindFreeRole(const FState& State, const uint8 Allowed)
	{
		int32 Best = INDEX_NONE;
		for (int32 Role = 0; Role < RoleCount; ++Role)
		{
			if (Allows(Allowed, Role) && State.Room(Role) > 0 && (Best == INDEX_NONE || State.Room(Role) > State.Room(Best)))
			{
				Best = Role;
			}
		}
		return Best;
	}

	/**
	 * Breadth-first search over roles: frees one of the Allowed (full) roles by moving assigned members along a chain of
	 * roles that ends in a role with room. Returns the freed role, or INDEX_NONE.
	 */
	int32 FreeRoleByShifting(FState& State, const uint8 Allowed)
	{
		TStaticArray<int32, RoleCount> ParentRole(InPlace, INDEX_NONE);
		TStaticArray<int32, RoleCount> MovedMember(InPlace, INDEX_NONE);
		TStaticArray<bool, RoleCount> Visited(InPlace, false);
		TArray<int32, TInlineAllocator<RoleCount>> Queue;
		for (int32 Role = 0; Role < RoleCount; ++Role)
		{
			if (Allows(Allowed, Role))
			{
				Visited[Role] = true;
				Queue.Add(Role);
			}
		}
		for (int32 Head = 0; Head < Queue.Num(); ++Head)
		{
			const int32 From = Queue[Head];
			for (int32 Member = 0; Member < State.Roles.Num(); ++Member)
			{
				if (State.Roles[Member] != From)
				{
					continue;
				}
				for (int32 To = 0; To < RoleCount; ++To)
				{
					if (Visited[To] || !Allows(AllowedOf(State.Candidates[Member]), To))
					{
						continue;
					}
					Visited[To] = true;
					ParentRole[To] = From;
					MovedMember[To] = Member;
					if (State.Room(To) > 0)
					{
						int32 Role = To;
						for (; ParentRole[Role] != INDEX_NONE; Role = ParentRole[Role])
						{
							State.Set(MovedMember[Role], Role);
						}
						return Role;
					}
					Queue.Add(To);
				}
			}
		}
		return INDEX_NONE;
	}

	/** Nobody can make room: the allowed role that is least over its quota. */
	int32 LeastOverfullRole(const FState& State, const uint8 Allowed)
	{
		int32 Best = INDEX_NONE;
		for (int32 Role = 0; Role < RoleCount; ++Role)
		{
			if (Allows(Allowed, Role) && (Best == INDEX_NONE || State.Room(Role) > State.Room(Best)))
			{
				Best = Role;
			}
		}
		return Best;
	}

	void FillUnassigned(FState& State)
	{
		TArray<int32> Pending;
		for (int32 Member = 0; Member < State.Roles.Num(); ++Member)
		{
			if (State.Roles[Member] == INDEX_NONE)
			{
				Pending.Add(Member);
			}
		}
		Algo::StableSortBy(Pending, [&State](const int32 Member) { return FMath::CountBits(AllowedOf(State.Candidates[Member])); });

		for (const int32 Member : Pending)
		{
			const uint8 Allowed = AllowedOf(State.Candidates[Member]);
			int32 Role = FindFreeRole(State, Allowed);
			if (Role == INDEX_NONE)
			{
				Role = FreeRoleByShifting(State, Allowed);
			}
			if (Role == INDEX_NONE)
			{
				Role = LeastOverfullRole(State, Allowed);
			}
			State.Set(Member, Role);
		}
	}
}

EnemyRoles::FQuotas EnemyRoles::ComputeQuotas(const int32 GroupSize, const FEnemyRoleWeights& Weights)
{
	FQuotas Quotas(InPlace, 0);
	if (GroupSize <= 0)
	{
		return Quotas;
	}
	float Total = 0.f;
	for (int32 Role = 0; Role < RoleCount; ++Role)
	{
		Total += FMath::Max(0.f, Weights.Get(EnemyRoleAssignment::RoleAt(Role)));
	}
	if (Total <= UE_KINDA_SMALL_NUMBER)
	{
		Quotas[0] = GroupSize;
		return Quotas;
	}

	TStaticArray<float, RoleCount> Remainders(InPlace, 0.f);
	int32 Left = GroupSize;
	for (int32 Role = 0; Role < RoleCount; ++Role)
	{
		const float Exact = GroupSize * FMath::Max(0.f, Weights.Get(EnemyRoleAssignment::RoleAt(Role))) / Total;
		Quotas[Role] = FMath::FloorToInt(Exact + UE_KINDA_SMALL_NUMBER);
		Remainders[Role] = Exact > 0.f ? Exact - Quotas[Role] : -1.f;
		Left -= Quotas[Role];
	}
	for (; Left > 0; --Left)
	{
		int32 Best = 0;
		for (int32 Role = 1; Role < RoleCount; ++Role)
		{
			Best = Remainders[Role] > Remainders[Best] ? Role : Best;
		}
		++Quotas[Best];
		Remainders[Best] = -1.f;
	}
	return Quotas;
}

TArray<EEnemyRole> EnemyRoles::Assign(const TArray<FCandidate>& Candidates, const FEnemyRoleWeights& Weights)
{
	EnemyRoleAssignment::FState State{Candidates, ComputeQuotas(Candidates.Num(), Weights), FQuotas(InPlace, 0), {}};
	State.Roles.Init(INDEX_NONE, Candidates.Num());
	EnemyRoleAssignment::KeepCurrentRoles(State);
	EnemyRoleAssignment::FillUnassigned(State);

	TArray<EEnemyRole> Roles;
	Roles.Reserve(Candidates.Num());
	for (const int32 Role : State.Roles)
	{
		Roles.Add(EnemyRoleAssignment::RoleAt(Role));
	}
	return Roles;
}
