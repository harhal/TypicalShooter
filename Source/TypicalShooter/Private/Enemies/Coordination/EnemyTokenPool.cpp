#include "Enemies/Coordination/EnemyTokenPool.h"

void FEnemyTokenPool::Update(const float Now, const TArray<FEnemyTokenCandidate>& Candidates, const int32 TokenCount,
	const FEnemyTokenRotationSettings& Settings)
{
	SyncMembers(Now, Candidates);
	ReleaseExpired(Now, Settings);
	ReleaseOverflow(Now, TokenCount);
	Grant(Now, TokenCount, Settings);
}

void FEnemyTokenPool::Release(const int32 Id, const float Now)
{
	if (FMemberState* State = Members.Find(Id); State && State->bHolding)
	{
		GiveBack(*State, Now);
	}
}

bool FEnemyTokenPool::HasToken(const int32 Id) const
{
	const FMemberState* State = Members.Find(Id);
	return State && State->bHolding;
}

TArray<int32> FEnemyTokenPool::GetHolders() const
{
	TArray<int32> Holders;
	for (const TPair<int32, FMemberState>& Pair : Members)
	{
		if (Pair.Value.bHolding)
		{
			Holders.Add(Pair.Key);
		}
	}
	return Holders;
}

void FEnemyTokenPool::SyncMembers(const float Now, const TArray<FEnemyTokenCandidate>& Candidates)
{
	TSet<int32> Present;
	for (const FEnemyTokenCandidate& Candidate : Candidates)
	{
		Present.Add(Candidate.Id);
		FMemberState* State = Members.Find(Candidate.Id);
		if (!State)
		{
			State = &Members.Add(Candidate.Id);
			State->WaitingSince = Now;
		}
		State->Last = Candidate;
		if (Candidate.bSeesTarget)
		{
			State->LostSightSince.Reset();
		}
		else if (!State->LostSightSince.IsSet())
		{
			State->LostSightSince = Now;
		}
	}
	for (auto It = Members.CreateIterator(); It; ++It)
	{
		if (!Present.Contains(It.Key()))
		{
			It.RemoveCurrent();
		}
	}
}

void FEnemyTokenPool::ReleaseExpired(const float Now, const FEnemyTokenRotationSettings& Settings)
{
	for (TPair<int32, FMemberState>& Pair : Members)
	{
		FMemberState& State = Pair.Value;
		if (!State.bHolding)
		{
			continue;
		}
		const bool bLostSight = State.LostSightSince.IsSet() && Now - State.LostSightSince.GetValue() >= Settings.LostSightGrace;
		const bool bHeldLongEnough = Now - State.HoldStart >= Settings.TokenHoldTime;
		if (bLostSight || (bHeldLongEnough && HasOtherClaimant(Pair.Key)))
		{
			GiveBack(State, Now);
		}
	}
}

void FEnemyTokenPool::ReleaseOverflow(const float Now, const int32 TokenCount)
{
	for (int32 Extra = CountHolders() - TokenCount; Extra > 0; --Extra)
	{
		FMemberState* Worst = nullptr;
		for (TPair<int32, FMemberState>& Pair : Members)
		{
			if (Pair.Value.bHolding && (!Worst || IsBefore(*Worst, Pair.Value)))
			{
				Worst = &Pair.Value;
			}
		}
		GiveBack(*Worst, Now);
	}
}

void FEnemyTokenPool::Grant(const float Now, const int32 TokenCount, const FEnemyTokenRotationSettings& Settings)
{
	for (int32 Free = TokenCount - CountHolders(); Free > 0; --Free)
	{
		FMemberState* Best = nullptr;
		bool bBestFresh = false;
		for (TPair<int32, FMemberState>& Pair : Members)
		{
			const FMemberState& State = Pair.Value;
			if (!IsClaimant(State))
			{
				continue;
			}
			// Enemies off cooldown go first; one on cooldown gets a token only when nobody else wants it.
			const bool bFresh = Now >= State.ReleasedAt + Settings.TokenCooldown;
			if (!Best || (bFresh && !bBestFresh) || (bFresh == bBestFresh && IsBefore(State, *Best)))
			{
				Best = &Pair.Value;
				bBestFresh = bFresh;
			}
		}
		if (!Best)
		{
			return;
		}
		Best->bHolding = true;
		Best->HoldStart = Now;
	}
}

void FEnemyTokenPool::GiveBack(FMemberState& State, const float Now)
{
	State.bHolding = false;
	State.ReleasedAt = Now;
	State.WaitingSince = Now;
}

bool FEnemyTokenPool::IsBefore(const FMemberState& A, const FMemberState& B)
{
	if (A.Last.Role != B.Last.Role)
	{
		return A.Last.Role < B.Last.Role;
	}
	if (A.WaitingSince != B.WaitingSince)
	{
		return A.WaitingSince < B.WaitingSince;
	}
	if (A.Last.DistanceToTarget != B.Last.DistanceToTarget)
	{
		return A.Last.DistanceToTarget < B.Last.DistanceToTarget;
	}
	return A.Last.Id < B.Last.Id;
}

bool FEnemyTokenPool::IsClaimant(const FMemberState& State)
{
	return !State.bHolding && State.Last.bSeesTarget;
}

bool FEnemyTokenPool::HasOtherClaimant(const int32 Except) const
{
	for (const TPair<int32, FMemberState>& Pair : Members)
	{
		if (Pair.Key != Except && IsClaimant(Pair.Value))
		{
			return true;
		}
	}
	return false;
}

int32 FEnemyTokenPool::CountHolders() const
{
	return GetHolders().Num();
}
