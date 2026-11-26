#include "Enemies/Coordination/EnemySectors.h"

#include "Algo/Count.h"
#include "Algo/StableSort.h"

namespace EnemySectorsImpl
{
	float Gap(const float A, const float B) { return FMath::Abs(FMath::FindDeltaAngleDegrees(A, B)); }

	/** Phase of N evenly spaced slots that best fits the current yaws (circular mean of N * yaw, divided by N). */
	float FitPhase(const FEnemySectorQuery& Query)
	{
		const int32 N = Query.Members.Num();
		double Sin = 0.0;
		double Cos = 0.0;
		for (const FEnemySectorMember& Member : Query.Members)
		{
			const double Radians = FMath::DegreesToRadians(double(Member.CurrentYaw) * N);
			Sin += FMath::Sin(Radians);
			Cos += FMath::Cos(Radians);
		}
		if (FMath::Abs(Sin) + FMath::Abs(Cos) < UE_KINDA_SMALL_NUMBER)
		{
			return Query.Members[0].CurrentYaw;
		}
		return float(FMath::RadiansToDegrees(FMath::Atan2(Sin, Cos)) / N);
	}

	TArray<float> MakeSlots(const float Phase, const int32 N)
	{
		TArray<float> Slots;
		for (int32 Index = 0; Index < N; ++Index)
		{
			Slots.Add(FRotator::NormalizeAxis(Phase + Index * 360.f / N));
		}
		return Slots;
	}

	bool IsOutOfView(const FEnemySectorQuery& Query, const float Yaw) { return Gap(Yaw, Query.ViewYaw) > Query.FlankViewHalfAngle; }

	int32 CountOutOfView(const FEnemySectorQuery& Query, const TArray<float>& Slots)
	{
		return Algo::CountIf(Slots, [&Query](const float Slot) { return IsOutOfView(Query, Slot); });
	}

	/** Greedy matching by angular distance: the closest member-slot pairs first. Only slots with Usable[slot] are taken. */
	void MatchNearest(const FEnemySectorQuery& Query, const TArray<int32>& Members, const TArray<float>& Slots,
		const TArray<bool>& Usable, TArray<bool>& Taken, TArray<int32>& Out)
	{
		struct FPair { int32 Member; int32 Slot; float Gap; };
		TArray<FPair> Pairs;
		for (const int32 Member : Members)
		{
			for (int32 Slot = 0; Slot < Slots.Num(); ++Slot)
			{
				if (Usable[Slot] && !Taken[Slot])
				{
					Pairs.Add({Member, Slot, Gap(Query.Members[Member].CurrentYaw, Slots[Slot])});
				}
			}
		}
		Algo::StableSortBy(Pairs, &FPair::Gap);

		TSet<int32> Done;
		for (const FPair& Pair : Pairs)
		{
			if (!Taken[Pair.Slot] && !Done.Contains(Pair.Member))
			{
				Taken[Pair.Slot] = true;
				Done.Add(Pair.Member);
				Out[Pair.Member] = Pair.Slot;
			}
		}
	}

	void SplitByRole(const FEnemySectorQuery& Query, TArray<int32>& Flankers, TArray<int32>& Others)
	{
		for (int32 Index = 0; Index < Query.Members.Num(); ++Index)
		{
			(Query.Members[Index].Role == EEnemyRole::Flank ? Flankers : Others).Add(Index);
		}
	}
}

TArray<float> EnemySectors::Assign(const FEnemySectorQuery& Query)
{
	using namespace EnemySectorsImpl;
	const int32 N = Query.Members.Num();
	if (N == 0)
	{
		return {};
	}
	TArray<int32> Flankers;
	TArray<int32> Others;
	SplitByRole(Query, Flankers, Others);

	TArray<float> Slots = MakeSlots(FitPhase(Query), N);
	if (CountOutOfView(Query, Slots) < Flankers.Num())
	{
		// Not enough room outside the view: put one slot right behind the target (most slots fall into the back arc).
		Slots = MakeSlots(Query.ViewYaw + 180.f, N);
	}

	TArray<bool> OutOfView;
	TArray<bool> AnySlot;
	for (const float Slot : Slots)
	{
		OutOfView.Add(IsOutOfView(Query, Slot));
		AnySlot.Add(true);
	}
	TArray<bool> Taken;
	Taken.Init(false, N);
	TArray<int32> SlotOf;
	SlotOf.Init(INDEX_NONE, N);

	MatchNearest(Query, Flankers, Slots, OutOfView, Taken, SlotOf);
	TArray<int32> Rest = Others;
	Rest.Append(Flankers.FilterByPredicate([&SlotOf](const int32 Member) { return SlotOf[Member] == INDEX_NONE; }));
	MatchNearest(Query, Rest, Slots, AnySlot, Taken, SlotOf);

	TArray<float> Yaws;
	for (int32 Member = 0; Member < N; ++Member)
	{
		Yaws.Add(Slots[SlotOf[Member]]);
	}
	return Yaws;
}

float EnemySectors::SectorHalfWidth(const int32 GroupSize)
{
	return GroupSize > 0 ? 180.f / GroupSize : 180.f;
}

float EnemySectors::YawAround(const FVector& Target, const FVector& Point)
{
	const FVector Offset = Point - Target;
	return FMath::RadiansToDegrees(FMath::Atan2(Offset.Y, Offset.X));
}

bool EnemySectors::ShouldReaimFlank(const float ReferenceViewYaw, const float ViewYaw, const float Threshold)
{
	return EnemySectorsImpl::Gap(ReferenceViewYaw, ViewYaw) > Threshold;
}
