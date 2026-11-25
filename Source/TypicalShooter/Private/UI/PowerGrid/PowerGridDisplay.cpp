#include "UI/PowerGrid/PowerGridDisplay.h"

#include "InventoryComponent.h"
#include "InventoryItem.h"
#include "PowerGrid/PowerEffectsFragment.h"
#include "PowerGrid/PowerGridComponent.h"
#include "PowerGrid/ReceiverAbilitiesComponent.h"

#define LOCTEXT_NAMESPACE "PowerGridDisplay"

namespace PowerGridDisplayPrivate
{
	template <typename FragmentType>
	const FragmentType* FindFragment(const UInventoryItem& Item)
	{
		const FInventoryItemManifest* Manifest = Item.GetItemManifest();
		return Manifest ? Manifest->GetFragmentOfType<FragmentType>() : nullptr;
	}

	TOptional<FIntPoint> GetClip(const UInventoryItem& Item, const FPowerGrid_ItemFragment& Fragment)
	{
		const bool bAmmoSource = Fragment.ItemType == EPowerItemType::Source && Fragment.Source.NeedsAmmo();
		return bAmmoSource ? FIntPoint(Item.GetClip(), Fragment.Source.Capacity) : TOptional<FIntPoint>();
	}

	FPowerCellDisplay MakeCell(const FPowerGridNode& Node, const FPowerGridNodeResult& Result, const UInventoryItem* Item)
	{
		FPowerCellDisplay Cell;
		Cell.Location = Node.Location;
		Cell.Type = Node.Type;
		Cell.OpenFaces = Node.OpenFaces;
		Cell.Power = Result.Power;
		Cell.bPowered = Result.bPowered;
		const FPowerGrid_ItemFragment* Fragment = Item ? FindFragment<FPowerGrid_ItemFragment>(*Item) : nullptr;
		if (Fragment)
		{
			Cell.RequiredPower = Node.Type == EPowerItemType::Receiver ? Fragment->Receiver.RequiredPower : 0;
			Cell.Clip = GetClip(*Item, *Fragment);
		}
		return Cell;
	}

	FText DescribePower(const UInventoryItem& Item, const FPowerGrid_ItemFragment& Fragment, const int32 Power)
	{
		FText Text = Fragment.ItemType == EPowerItemType::Receiver
			? FText::Format(LOCTEXT("ReceiverPower", "Power {0} / {1}"), Power, Fragment.Receiver.RequiredPower)
			: FText::Format(LOCTEXT("Power", "Power {0}"), Power);
		if (const TOptional<FIntPoint> Clip = GetClip(Item, Fragment))
		{
			Text = FText::Format(LOCTEXT("PowerAndClip", "{0}, clip {1}/{2}"), Text, FText::AsNumber(Clip->X), FText::AsNumber(Clip->Y));
		}
		return Text;
	}

	FText DescribeStep(const FPowerThresholdEffect& Step)
	{
		TArray<FText> Parts;
		for (const FAttackStatBonus& Bonus : Step.AttackBonuses)
		{
			const FText Name = StaticEnum<EAttackAttribute>()->GetDisplayNameTextByValue(static_cast<int64>(Bonus.Attribute));
			const FText Sign = Bonus.Value >= 0.f ? FText::FromString(TEXT("+")) : FText::GetEmpty();
			Parts.Add(FText::Format(LOCTEXT("Bonus", "{0} {1}{2}"), Name, Sign, FText::AsNumber(Bonus.Value)));
		}
		if (Step.CharacterEffect)
		{
			Parts.Add(Step.CharacterEffect->GetDisplayNameText());
		}
		return FText::Format(LOCTEXT("Step", "Power {0}+: {1}"), Step.Threshold, FText::Join(FText::FromString(TEXT(", ")), Parts));
	}
}

const FPowerCellDisplay* FPowerGridDisplay::FindCell(const FHhAxialCoord& Location) const
{
	return Cells.FindByPredicate([&Location](const FPowerCellDisplay& Cell) { return Cell.Location == Location; });
}

bool FPowerGridDisplay::operator==(const FPowerGridDisplay& Other) const
{
	auto SameCell = [](const FPowerCellDisplay& A, const FPowerCellDisplay& B)
	{
		return A.Location == B.Location && A.Type == B.Type && A.OpenFaces == B.OpenFaces && A.Power == B.Power
			&& A.bPowered == B.bPowered && A.RequiredPower == B.RequiredPower && A.Clip == B.Clip;
	};
	auto SameLink = [](const FPowerLinkDisplay& A, const FPowerLinkDisplay& B)
	{
		return A.From == B.From && A.To == B.To && A.bPowered == B.bPowered;
	};
	if (bShowSlots != Other.bShowSlots || Cells.Num() != Other.Cells.Num() || Links.Num() != Other.Links.Num())
	{
		return false;
	}
	for (int32 Index = 0; Index < Cells.Num(); ++Index)
	{
		if (!SameCell(Cells[Index], Other.Cells[Index]))
		{
			return false;
		}
	}
	for (int32 Index = 0; Index < Links.Num(); ++Index)
	{
		if (!SameLink(Links[Index], Other.Links[Index]))
		{
			return false;
		}
	}
	return true;
}

namespace PowerGridDisplay
{
	FPowerGridDisplay Build(const UPowerGridComponent& Grid)
	{
		using namespace PowerGridDisplayPrivate;
		FPowerGridDisplay Display;
		const TArray<FPowerGridNode>& Nodes = Grid.GetNodes();
		const FPowerGridSolution& Solution = Grid.GetSolution();
		const TArray<UInventoryItem*> Items = Grid.GetNodeItems();
		for (int32 Index = 0; Index < Nodes.Num() && Index < Solution.Nodes.Num(); ++Index)
		{
			Display.Cells.Add(MakeCell(Nodes[Index], Solution.Nodes[Index], Items.IsValidIndex(Index) ? Items[Index] : nullptr));
		}
		for (const FPowerGridEdge& Edge : Solution.Edges)
		{
			Display.Links.Add({ Nodes[Edge.From].Location, Nodes[Edge.To].Location, Edge.bPowered });
		}
		const AActor* Owner = Grid.GetOwner();
		Display.bShowSlots = Owner && Owner->FindComponentByClass<UReceiverAbilitiesComponent>();
		return Display;
	}

	FItemPowerInfo DescribeItem(const UInventoryItem& Item, const UPowerGridComponent* Grid)
	{
		using namespace PowerGridDisplayPrivate;
		FItemPowerInfo Info;
		const int32 Power = Grid ? Grid->GetItemPower(&Item) : 0;
		if (const FPowerGrid_ItemFragment* Fragment = FindFragment<FPowerGrid_ItemFragment>(Item))
		{
			Info.TypeText = StaticEnum<EPowerItemType>()->GetDisplayNameTextByValue(static_cast<int64>(Fragment->ItemType));
			Info.PowerText = DescribePower(Item, *Fragment, Power);
		}
		if (const FPowerEffects_ItemFragment* Effects = FindFragment<FPowerEffects_ItemFragment>(Item))
		{
			for (const FPowerThresholdEffect& Step : Effects->Effects)
			{
				// Outside the grid the power is 0: only threshold 0 is reached (R13).
				Info.Steps.Add({ DescribeStep(Step), Step.Threshold <= Power });
			}
		}
		return Info;
	}

	UPowerGridComponent* FindGridOf(const UInventoryItem& Item)
	{
		const UInventoryComponent* Inventory = Item.GetTypedOuter<UInventoryComponent>();
		const AActor* Owner = Inventory ? Inventory->GetOwner() : nullptr;
		TArray<UPowerGridComponent*> Grids;
		if (Owner)
		{
			Owner->GetComponents(Grids);
		}
		UPowerGridComponent* const* Found = Grids.FindByPredicate([Inventory](const UPowerGridComponent* Grid) { return Grid->GetInventory() == Inventory; });
		return Found ? *Found : nullptr;
	}
}

#undef LOCTEXT_NAMESPACE
