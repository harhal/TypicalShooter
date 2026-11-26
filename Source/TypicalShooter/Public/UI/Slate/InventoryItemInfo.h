// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/InventoryItem/InventoryItem_View.h"
#include "UI/PowerGrid/PowerGridDisplay.h"
#include "Widgets/SCompoundWidget.h"

class SImage;
class STextBlock;
class SVerticalBox;

/**
 * Item details panel: large icon, name and description. Empty (collapsed content) when no item is shown.
 */
class TYPICALSHOOTER_API SInventoryItemInfo : public SCompoundWidget, public IInventoryItem_View
{
public:
	SLATE_BEGIN_ARGS(SInventoryItemInfo)
		: _IconSize(128.f)
	{ }
		SLATE_ARGUMENT(float, IconSize)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual void SetIcon(const FSlateBrush& InIconBrush) override;
	virtual void SetName(const FText& InName) override;
	virtual void SetDescription(const FText& InDescription) override;
	virtual void SetCount(int32 InCount) override;
	virtual void SetPowerInfo(const FItemPowerInfo& Info) override;
	virtual void SetRarity(const TOptional<FItemRarityDisplay>& InRarity) override;

	bool HasItem() const { return Count > 0; }
	FText GetDisplayedName() const;
	FLinearColor GetNameColor() const;
	/** Rarity line under the name; empty when the item has no rarity. */
	FText GetDisplayedRarity() const;
	FText GetDisplayedDescription() const;
	const FItemPowerInfo& GetPowerInfo() const { return PowerInfo; }
	/** Text lines of the power section: type, power, one per effect step (0 when hidden). */
	int32 GetNumPowerLines() const { return NumPowerLines; }
	bool IsPowerStepHighlighted(int32 StepIndex) const;

private:
	FSlateBrush IconBrush;
	int32 Count = 0;

	TSharedPtr<SImage> IconImage;
	TSharedPtr<STextBlock> NameText;
	TSharedPtr<STextBlock> RarityText;
	TOptional<FItemRarityDisplay> Rarity;
	TSharedPtr<STextBlock> DescriptionText;

	void RebuildPowerLines();

	FItemPowerInfo PowerInfo;
	int32 NumPowerLines = 0;
	TSharedPtr<SVerticalBox> PowerLines;
	FLinearColor ReachedStepColor = FLinearColor(1.f, 0.8f, 0.15f, 1.f);
	FLinearColor PendingStepColor = FLinearColor(0.6f, 0.6f, 0.65f, 1.f);
};
