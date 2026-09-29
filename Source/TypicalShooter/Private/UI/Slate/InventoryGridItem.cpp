// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Slate/InventoryGridItem.h"

#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SInventoryGridItem::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SButton)
		.OnHovered(this, &SInventoryGridItem::HandleHovered, true)
		.OnUnhovered(this, &SInventoryGridItem::HandleHovered, false)
		.OnReceivedFocus(this, &SInventoryGridItem::HandleFocus, true)
		.OnLostFocus(this, &SInventoryGridItem::HandleFocus, false)
		.OnSlateButtonDragDetected(this, &SInventoryGridItem::HandleDrag, true)
		.AllowDragDrop(true)
		.ButtonStyle(FCoreStyle::Get(), "NoBorder")
		[
			SNew(SVerticalBox)
 
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SAssignNew(IconBrushWidget, SImage)
			]
 
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(FMargin(0.f, 4.f, 0.f, 0.f))
			[
				SAssignNew(NameTextBlock, STextBlock)
				.Text(FText::GetEmpty())
			]
		]
	];
}

void SInventoryGridItem::HandleHovered(bool bHovered)
{
}

void SInventoryGridItem::HandleFocus(bool bFocus)
{
}

FReply SInventoryGridItem::HandleDrag(const FGeometry& Geometry, const FPointerEvent& PointerEvent, bool bDrag)
{
	return FReply::Handled();
}

void SInventoryGridItem::SetIcon(FSlateBrush IconBrush)
{
	IconBrushWidget->SetImage(&IconBrush);
}

void SInventoryGridItem::SetName(FString Name)
{
	NameTextBlock->SetText(*Name);
}

void SInventoryGridItem::SetDescription(FString Description)
{
}

void SInventoryGridItem::SetCount(int32 Count)
{
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
