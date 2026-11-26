// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Slate/InventoryItemInfo.h"

#include "SlateOptMacros.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Text/STextBlock.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SInventoryItemInfo::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)
		.Visibility_Lambda([this]() { return HasItem() ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed; })

		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(InArgs._IconSize)
			.HeightOverride(InArgs._IconSize)
			[
				SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFit)
				[
					SAssignNew(IconImage, SImage)
					.Image(&IconBrush)
				]
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(0.f, 8.f, 0.f, 4.f))
		[
			SAssignNew(NameText, STextBlock)
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
			.ColorAndOpacity_Lambda([this]() { return FSlateColor(GetNameColor()); })
			.AutoWrapText(true)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(0.f, 0.f, 0.f, 4.f))
		[
			SAssignNew(RarityText, STextBlock)
			.Font(FCoreStyle::GetDefaultFontStyle("Italic", 11))
			.ColorAndOpacity_Lambda([this]() { return FSlateColor(GetNameColor()); })
			.Visibility_Lambda([this]() { return Rarity ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(DescriptionText, STextBlock)
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
			.AutoWrapText(true)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(0.f, 8.f, 0.f, 0.f))
		[
			SAssignNew(PowerLines, SVerticalBox)
		]
	];
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SInventoryItemInfo::SetIcon(const FSlateBrush& InIconBrush)
{
	IconBrush = InIconBrush;
	IconImage->SetImage(&IconBrush);
	IconImage->SetVisibility(IconBrush.GetResourceObject() ? EVisibility::HitTestInvisible : EVisibility::Hidden);
}

void SInventoryItemInfo::SetName(const FText& InName)
{
	NameText->SetText(InName);
}

void SInventoryItemInfo::SetDescription(const FText& InDescription)
{
	DescriptionText->SetText(InDescription);
}

void SInventoryItemInfo::SetCount(const int32 InCount)
{
	Count = InCount;
}

FText SInventoryItemInfo::GetDisplayedName() const
{
	return NameText->GetText();
}

FText SInventoryItemInfo::GetDisplayedDescription() const
{
	return DescriptionText->GetText();
}

void SInventoryItemInfo::SetPowerInfo(const FItemPowerInfo& Info)
{
	PowerInfo = Info;
	RebuildPowerLines();
}

void SInventoryItemInfo::RebuildPowerLines()
{
	PowerLines->ClearChildren();
	NumPowerLines = 0;
	auto AddLine = [this](const FText& Text, const FSlateFontInfo& Font, const FLinearColor& Color)
	{
		PowerLines->AddSlot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(Text)
			.Font(Font)
			.ColorAndOpacity(Color)
			.AutoWrapText(true)
		];
		++NumPowerLines;
	};
	if (!PowerInfo.TypeText.IsEmpty())
	{
		AddLine(PowerInfo.TypeText, FCoreStyle::GetDefaultFontStyle("Bold", 11), FLinearColor::White);
		AddLine(PowerInfo.PowerText, FCoreStyle::GetDefaultFontStyle("Regular", 11), FLinearColor::White);
	}
	for (int32 Index = 0; Index < PowerInfo.Steps.Num(); ++Index)
	{
		// Reached steps stand out; the rest are dimmed (R16).
		const bool bReached = IsPowerStepHighlighted(Index);
		AddLine(PowerInfo.Steps[Index].Text, FCoreStyle::GetDefaultFontStyle(bReached ? "Bold" : "Regular", 10),
			bReached ? ReachedStepColor : PendingStepColor);
	}
}

bool SInventoryItemInfo::IsPowerStepHighlighted(const int32 StepIndex) const
{
	return PowerInfo.Steps.IsValidIndex(StepIndex) && PowerInfo.Steps[StepIndex].bReached;
}

void SInventoryItemInfo::SetRarity(const TOptional<FItemRarityDisplay>& InRarity)
{
	Rarity = InRarity;
	RarityText->SetText(Rarity ? Rarity->Name : FText::GetEmpty());
}

FLinearColor SInventoryItemInfo::GetNameColor() const
{
	return Rarity ? Rarity->Color : FLinearColor::White;
}

FText SInventoryItemInfo::GetDisplayedRarity() const
{
	return RarityText->GetText();
}
