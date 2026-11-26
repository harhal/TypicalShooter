#include "Loot/LootItemText.h"

#include "Loot/LootItemTemplate.h"
#include "Loot/LootModifier.h"
#include "Loot/LootRarity.h"

namespace LootItemTextPrivate
{
	FText Number(const float Value)
	{
		FNumberFormattingOptions Options;
		Options.MaximumFractionalDigits = 2;
		Options.UseGrouping = false;
		return FText::AsNumber(Value, &Options);
	}

	FFormatNamedArguments ItemArguments(const FPowerGrid_ItemFragment& PowerGrid, const int32 Level, const ULootRarity* Rarity)
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("Power"), PowerGrid.Source.Power);
		Args.Add(TEXT("Capacity"), PowerGrid.Source.Capacity);
		Args.Add(TEXT("CostPerUse"), PowerGrid.Source.CostPerUse);
		Args.Add(TEXT("RechargeDelay"), Number(PowerGrid.Source.RechargeDelay));
		Args.Add(TEXT("RequiredPower"), PowerGrid.Receiver.RequiredPower);
		Args.Add(TEXT("Level"), Level);
		Args.Add(TEXT("Rarity"), Rarity ? Rarity->DisplayName : FText::GetEmpty());
		return Args;
	}

	FText ModifierLine(const FLootRolledModifier& Rolled)
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("Threshold"), Rolled.Step.Threshold);
		for (int32 Index = 0; Index < Rolled.Step.AttackBonuses.Num(); ++Index)
		{
			Args.Add(FString::Printf(TEXT("Bonus%d"), Index), Number(Rolled.Step.AttackBonuses[Index].Value));
		}
		return FText::Format(Rolled.Modifier->DescriptionLine, Args);
	}

	FText Description(const ULootItemTemplate& Template, const FPowerGrid_ItemFragment& PowerGrid,
		TConstArrayView<FLootRolledModifier> Rolled, const int32 Level, const ULootRarity* Rarity)
	{
		TArray<FText> Lines;
		if (!Template.DescriptionFormat.IsEmpty())
		{
			Lines.Add(FText::Format(Template.DescriptionFormat, ItemArguments(PowerGrid, Level, Rarity)));
		}
		for (const FLootRolledModifier& Entry : Rolled)
		{
			if (Entry.Modifier && !Entry.Modifier->DescriptionLine.IsEmpty())
			{
				Lines.Add(ModifierLine(Entry));
			}
		}
		return FText::Join(FText::FromString(TEXT("\n")), Lines);
	}

	UTexture2D* Icon(const ULootItemTemplate& Template, const ULootRarity* Rarity)
	{
		const TObjectPtr<UTexture2D>* ByRarity = Rarity ? Template.IconsByRarity.Find(Rarity) : nullptr;
		return ByRarity && *ByRarity ? ByRarity->Get() : Template.DefaultIcon.Get();
	}
}

FInfo_ItemFragment LootItemText::MakeInfo(const ULootItemTemplate& Template, const FPowerGrid_ItemFragment& PowerGrid,
	TConstArrayView<FLootRolledModifier> Rolled, const int32 Level, const ULootRarity* Rarity)
{
	FInfo_ItemFragment Info;
	Info.DisplayName = Template.DisplayName;
	Info.Description = LootItemTextPrivate::Description(Template, PowerGrid, Rolled, Level, Rarity);
	Info.Icon = LootItemTextPrivate::Icon(Template, Rarity);
	return Info;
}
