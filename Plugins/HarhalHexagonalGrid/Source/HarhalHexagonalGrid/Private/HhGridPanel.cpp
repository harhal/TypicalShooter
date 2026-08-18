// Fill out your copyright notice in the Description page of Project Settings.

#include "HhGridPanel.h"
#include "HhAxialCoord.h"
#include "HhGridFunctions.h"

void SHhGridPanel::Construct(const FArguments& InArgs)
{
    HexSize = InArgs._HexSize > 0.f ? InArgs._HexSize : 50.f;
    HorizontalSize = InArgs._HorizontalSize > 0 ? InArgs._HorizontalSize : 1;
    VerticalSize = InArgs._VerticalSize > 0 ? InArgs._VerticalSize : 1;
}
 
TPanelChildren<SHhGridPanel::FSlot>::FScopedWidgetSlotArguments SHhGridPanel::AddSlot(const FHhAxialCoord& Coord)
{
    return TPanelChildren<SHhGridPanel::FSlot>::FScopedWidgetSlotArguments{ MakeUnique<FSlot>(Coord), Children, INDEX_NONE };
}
 
void SHhGridPanel::RemoveSlot(const FHhAxialCoord& Coord)
{
    for (int32 i = Children.Num() - 1; i >= 0; --i)
    {
        if (Children[i].GetCoord() == Coord)
        {
            Children.RemoveAt(i);
            Invalidate(EInvalidateWidgetReason::Layout);
            break;
        }
    }
}

void SHhGridPanel::ClearChildren()
{
    Children.Empty();
}
 
void SHhGridPanel::SetHexSize(float NewSize)
{
    HexSize = NewSize;
    Invalidate(EInvalidateWidgetReason::Layout);
}

void SHhGridPanel::SetGridSize(int32 InVerticalSize, int32 InHorizontalSize)
{
    VerticalSize = InVerticalSize;
    HorizontalSize = InHorizontalSize;
    Invalidate(EInvalidateWidgetReason::Layout);
}

void SHhGridPanel::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const
{
    const FVector2D Center = AllottedGeometry.GetLocalSize() * 0.5f;
 
    for (int32 i = 0; i < Children.Num(); ++i)
    {
        const FSlot& CurSlot = Children[i];
        const TSharedRef<SWidget>& Widget = CurSlot.GetWidget();
 
        if (Widget->GetVisibility() == EVisibility::Collapsed)
        {
            continue;
        }
 
        const FVector2D Point = UHhGridFunctions::HexAxialCoordToPoint(CurSlot.GetCoord(), HexSize) + Center;
        const FVector2D DesiredWSize = FVector2D(FMath::Sqrt(3.f) * HexSize, HexSize * 2.f);
 
        ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(
            Widget,
            Point - DesiredWSize * 0.5f,
            DesiredWSize
        ));
    }
}
 
FVector2D SHhGridPanel::ComputeDesiredSize(float) const
{    
    return FVector2D(FMath::Sqrt(3.f) * HexSize * HorizontalSize, 
        HexSize * (1.5f * VerticalSize + .5f));
}
 
FChildren* SHhGridPanel::GetChildren()
{
    return &Children;
}