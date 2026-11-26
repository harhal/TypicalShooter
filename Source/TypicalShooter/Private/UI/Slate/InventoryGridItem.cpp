// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Slate/InventoryGridItem.h"

#include "Rendering/DrawElements.h"
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

void SInventoryGridItem::SetRarity(const TOptional<FItemRarityDisplay>& InRarity)
{
	FrameColor = InRarity ? TOptional<FLinearColor>(InRarity->Color) : TOptional<FLinearColor>();
	Invalidate(EInvalidateWidgetReason::Paint);
}

namespace InventoryGridItemPaint
{
	constexpr float FrameThickness = 2.f;

	/** Closed outline of the pointy-top hex inscribed in Size, inset by Inset. */
	TArray<FVector2D> HexOutline(const FVector2D& Size, const float Inset)
	{
		const FVector2D Center = Size * 0.5f;
		const float Radius = FMath::Min(Size.Y * 0.5f, Size.X / UE_SQRT_3) - Inset;
		TArray<FVector2D> Points;
		for (int32 Corner = 0; Corner <= 6; ++Corner)
		{
			const float Angle = FMath::DegreesToRadians(60.f * Corner - 30.f);
			Points.Add(Center + Radius * FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)));
		}
		return Points;
	}
}

int32 SInventoryGridItem::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements, const int32 LayerId, const FWidgetStyle& InWidgetStyle, const bool bParentEnabled) const
{
	if (FrameColor)
	{
		using namespace InventoryGridItemPaint;
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(),
			HexOutline(AllottedGeometry.GetLocalSize(), FrameThickness), ESlateDrawEffect::None, *FrameColor, true, FrameThickness);
	}
	return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId + 1, InWidgetStyle, bParentEnabled);
}
