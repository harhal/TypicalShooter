// SInventoryGrid: the power grid drawn over the items (power grid R16).

#include "UI/Slate/InventoryGrid.h"

#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "UI/Slate/PowerGridOverlay.h"

namespace InventoryGridPowerPaint
{
	const FLinearColor PoweredColor(1.f, 0.8f, 0.15f, 1.f);
	const FLinearColor UnpoweredColor(0.55f, 0.55f, 0.6f, 0.8f);
	const FLinearColor SlotColor(0.4f, 0.85f, 1.f, 0.9f);

	void DrawText(FSlateWindowElementList& Elements, const int32 Layer, const FGeometry& Geometry, const FText& Text,
		const FSlateFontInfo& Font, const FVector2D& Center, const FLinearColor& Color)
	{
		const TSharedRef<FSlateFontMeasure> Measure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
		const FVector2D Size = Measure->Measure(Text, Font);
		FSlateDrawElement::MakeText(Elements, Layer, Geometry.ToPaintGeometry(Size, FSlateLayoutTransform(Center - Size * 0.5f)),
			Text, Font, ESlateDrawEffect::None, Color);
	}

	void DrawLine(FSlateWindowElementList& Elements, const int32 Layer, const FGeometry& Geometry,
		const FVector2D& From, const FVector2D& To, const FLinearColor& Color, const float Thickness)
	{
		FSlateDrawElement::MakeLines(Elements, Layer, Geometry.ToPaintGeometry(), { From, To }, ESlateDrawEffect::None, Color, true, Thickness);
	}
}

void SInventoryGrid::SetPowerGrid(const FPowerGridDisplay& Display)
{
	if (!(PowerGrid == Display))
	{
		PowerGrid = Display;
		Invalidate(EInvalidateWidgetReason::Paint);
	}
}

int32 SInventoryGrid::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements, const int32 LayerId, const FWidgetStyle& InWidgetStyle, const bool bParentEnabled) const
{
	const int32 ChildrenLayer = SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	return PaintPowerGrid(AllottedGeometry, OutDrawElements, ChildrenLayer + 1);
}

int32 SInventoryGrid::PaintPowerGrid(const FGeometry& Geometry, FSlateWindowElementList& OutDrawElements, const int32 LayerId) const
{
	using namespace InventoryGridPowerPaint;
	if (!FSlateApplication::IsInitialized())
	{
		return LayerId;
	}
	const FVector2D Size = Geometry.GetLocalSize();
	auto Center = [this, &Size](const FHhAxialCoord& Coord) { return PowerGridOverlay::GetCellCenter(Coord, HexSize, Size); };

	// Links: a short stroke across the shared edge, from one centre side to the other.
	for (const FPowerLinkDisplay& Link : PowerGrid.Links)
	{
		const FVector2D From = Center(Link.From);
		const FVector2D To = Center(Link.To);
		DrawLine(OutDrawElements, LayerId, Geometry, FMath::Lerp(From, To, 0.35f), FMath::Lerp(From, To, 0.65f),
			Link.bPowered ? PoweredColor : UnpoweredColor, Link.bPowered ? 4.f : 2.f);
	}

	const FSlateFontInfo Font = FCoreStyle::GetDefaultFontStyle("Bold", FMath::Max(6, FMath::RoundToInt(HexSize * 0.18f)));
	for (const FPowerCellDisplay& Cell : PowerGrid.Cells)
	{
		const FVector2D CellCenter = Center(Cell.Location);
		const FLinearColor Color = Cell.bPowered ? PoweredColor : UnpoweredColor;
		for (int32 Index = 0; Index < HexFaces::Num; ++Index)
		{
			const EHexFace Face = HexFaces::FromIndex(Index);
			if (HexFaces::IsOpen(Cell.OpenFaces, Face))
			{
				const TPair<FVector2D, FVector2D> Segment = PowerGridOverlay::GetFaceSegment(CellCenter, HexSize, Face);
				DrawLine(OutDrawElements, LayerId, Geometry, Segment.Key, Segment.Value, Color, 3.f);
			}
		}
		DrawText(OutDrawElements, LayerId + 1, Geometry, PowerGridOverlay::GetPowerLabel(Cell), Font,
			CellCenter + FVector2D(0.f, HexSize * 0.15f), Color); // above the item name
	}

	if (PowerGrid.bShowSlots)
	{
		for (const EReceiverSlot Slot : ReceiverSlots::GetAll())
		{
			const FHhAxialCoord Location = ReceiverSlots::GetLocation(Slot);
			DrawText(OutDrawElements, LayerId + 1, Geometry, PowerGridOverlay::GetSlotLabel(Location), Font,
				Center(Location) - FVector2D(0.f, HexSize * 0.5f), SlotColor);
		}
	}
	return LayerId + 1;
}
