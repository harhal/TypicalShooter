// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Slate/InventoryGridItem.h"

#include "Rendering/DrawElements.h"
#include "SlateOptMacros.h"
#include "Styling/CoreStyle.h"
#include "UI/Slate/PowerGridOverlay.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Text/STextBlock.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SInventoryGridItem::Construct(const FArguments& InArgs)
{
	SetVisibility(EVisibility::HitTestInvisible);

	// The slot is the hex's bounding box; keep the content inside the hex. The icon sits in the centre, over the
	// underlay (LootGeneration R11); the name stays at the bottom.
	ChildSlot
	.Padding(FMargin(12.f, 14.f))
	[
		SNew(SOverlay)

		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(FMargin(4.f))
		[
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			[
				SAssignNew(IconBrushWidget, SImage)
				.Image(&IconBrush)
			]
		]

		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Bottom)
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

void SInventoryGridItem::SetUnderlay(const TOptional<FItemUnderlay>& InUnderlay)
{
	if (Underlay != InUnderlay)
	{
		Underlay = InUnderlay;
		Invalidate(EInvalidateWidgetReason::Paint);
	}
}

namespace InventoryGridItemPaint
{
	constexpr float FrameThickness = 2.f;
	/** Spoke width as a fraction of the hex size. */
	constexpr float SpokeWidth = 0.22f;

	float HexSizeOf(const FVector2D& Size)
	{
		return FMath::Min(Size.Y * 0.5f, Size.X / UE_SQRT_3);
	}

	/** Spokes from the centre to every open face: the same picture for every item with the same faces. */
	void PaintUnderlay(const FItemUnderlay& Underlay, const FGeometry& Geometry, FSlateWindowElementList& Elements, const int32 Layer)
	{
		const FVector2D Size = Geometry.GetLocalSize();
		const float HexSize = HexSizeOf(Size);
		const FLinearColor Color = Underlay.bPowered ? PowerGridOverlay::PoweredColor : PowerGridOverlay::UnpoweredColor;
		for (int32 Index = 0; Index < HexFaces::Num; ++Index)
		{
			const EHexFace Face = HexFaces::FromIndex(Index);
			if (HexFaces::IsOpen(Underlay.OpenFaces, Face))
			{
				const TPair<FVector2D, FVector2D> Spoke = PowerGridOverlay::GetFaceSpoke(Size * 0.5f, HexSize, Face);
				FSlateDrawElement::MakeLines(Elements, Layer, Geometry.ToPaintGeometry(), { Spoke.Key, Spoke.Value },
					ESlateDrawEffect::None, Color, true, HexSize * SpokeWidth);
			}
		}
	}

	/** Closed outline of the pointy-top hex inscribed in Size, inset by Inset. */
	TArray<FVector2D> HexOutline(const FVector2D& Size, const float Inset)
	{
		const FVector2D Center = Size * 0.5f;
		const float Radius = HexSizeOf(Size) - Inset;
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
	using namespace InventoryGridItemPaint;
	if (Underlay)
	{
		PaintUnderlay(*Underlay, AllottedGeometry, OutDrawElements, LayerId);
	}
	if (FrameColor)
	{
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(),
			HexOutline(AllottedGeometry.GetLocalSize(), FrameThickness), ESlateDrawEffect::None, *FrameColor, true, FrameThickness);
	}
	return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId + 1, InWidgetStyle, bParentEnabled);
}
