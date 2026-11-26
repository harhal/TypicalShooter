#include "Loot/LootModifierPicker.h"

#include "Loot/LootItemTemplate.h"
#include "Loot/LootModifier.h"
#include "Loot/LootRandom.h"
#include "Loot/LootRarity.h"
#include "TypicalShooter.h"

namespace LootModifierPickerPrivate
{
	bool Fits(const ULootModifier& Modifier, const FLootModifierSlot& Slot)
	{
		return Modifier.Tier == Slot.Tier && (!Slot.Tag.IsValid() || Modifier.Tag.MatchesTag(Slot.Tag));
	}

	/** Pool weights with zero for entries that don't fit the slot or are already taken. */
	TArray<float> SlotWeights(TConstArrayView<FLootModifierEntry> Pool, const FLootModifierSlot& Slot, const TArray<const ULootModifier*>& Taken)
	{
		TArray<float> Weights;
		for (const FLootModifierEntry& Entry : Pool)
		{
			const bool bCandidate = Entry.Modifier && Fits(*Entry.Modifier, Slot) && !Taken.Contains(Entry.Modifier.Get());
			Weights.Add(bCandidate ? Entry.Weight : 0.f);
		}
		return Weights;
	}
}

TArray<const ULootModifier*> LootModifierPicker::Pick(TConstArrayView<FLootModifierEntry> Pool, const ULootRarity* Rarity, FRandomStream& Stream)
{
	TArray<const ULootModifier*> Picked;
	if (!Rarity || Pool.IsEmpty())
	{
		return Picked; // A template without a pool never gets modifiers: not a content error.
	}
	for (const FLootModifierSlot& Slot : Rarity->Slots)
	{
		const int32 Index = LootRandom::PickWeightedIndex(Stream, LootModifierPickerPrivate::SlotWeights(Pool, Slot, Picked));
		if (Index == INDEX_NONE)
		{
			UE_LOG(LogTypicalShooter, Warning, TEXT("Loot: rarity %s has no modifier for slot (tier %d, tag %s), skipped"),
				*GetNameSafe(Rarity), Slot.Tier, *Slot.Tag.ToString());
			continue;
		}
		Picked.Add(Pool[Index].Modifier.Get());
	}
	return Picked;
}
