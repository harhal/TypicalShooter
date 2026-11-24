// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Slate/InventoryGridItem.h"

#include "SlateOptMacros.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Text/STextBlock.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SInventoryGridItem::Construct(const FArguments& InArgs)
{
	SetVisibility(EVisibility::HitTestInvisible);

	// The slot is the hex's bounding box; keep the content inside the hex.
	ChildSlot
	.Padding(FMargin(12.f, 14.f))
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			[
				SAssignNew(IconBrushWidget, SImage)
				.Image(&IconBrush)
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		[
			SAssignNew(NameTextBlock, STextBlock)
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 7))
			.ShadowOffset(FVector2D(1.f, 1.f))
			.Clipping(EWidgetClipping::ClipToBounds)
			.Text(FText::GetEmpty())
		]
	];
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SInventoryGridItem::SetIcon(const FSlateBrush& InIconBrush)
{
	IconBrush = InIconBrush;
	IconBrushWidget->SetImage(&IconBrush);
	IconBrushWidget->SetVisibility(IconBrush.GetResourceObject() ? EVisibility::HitTestInvisible : EVisibility::Collapsed);
}

void SInventoryGridItem::SetName(const FText& InName)
{
	NameTextBlock->SetText(InName);
}

void SInventoryGridItem::SetDescription(const FText& InDescription)
{
	// Shown by the item info panel, not on the grid.
}

void SInventoryGridItem::SetCount(int32 InCount)
{
	Count = InCount;
}
