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
			.AutoWrapText(true)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(DescriptionText, STextBlock)
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
			.AutoWrapText(true)
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
