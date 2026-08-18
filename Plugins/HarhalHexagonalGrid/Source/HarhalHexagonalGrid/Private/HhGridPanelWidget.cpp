#include "HhGridPanelWidget.h"
#include "HhGridPanel.h"

FHhAxialCoord UHhGridPanelSlotWidget::GetCoord() const
{
    return Coord;
}

void UHhGridPanelSlotWidget::SetCoord(const FHhAxialCoord& InCoord)
{
    Coord = InCoord;
    
    if (!Slot)
    {
        return;
    }
    
    Slot->SetCoord(InCoord);
}

void UHhGridPanelSlotWidget::SynchronizeProperties()
{
    Super::SynchronizeProperties();
    
    SetCoord(Coord);
}

void UHhGridPanelSlotWidget::ReleaseSlateResources(bool bReleaseChildren)
{
    Super::ReleaseSlateResources(bReleaseChildren);
    
	Slot = nullptr;
}
 
void UHhGridPanelSlotWidget::BuildSlot(TSharedRef<SHhGridPanel> GridPanel)
{
    GridPanel->AddSlot(Coord)
        .Expose(Slot)
        [
            Content ? Content->TakeWidget() : SNullWidget::NullWidget
        ];
}

void UHhGridPanelSlotWidget::SynchronizeFromTemplate(const UPanelSlot* const TemplateSlot)
{
    if (const UHhGridPanelSlotWidget* TemplateHexSlot = Cast<UHhGridPanelSlotWidget>(TemplateSlot))
    {
        SetCoord(TemplateHexSlot->GetCoord());
    }
}

UHhGridPanelSlotWidget* UHhGridPanelWidget::AddChildToGrid(UWidget* Content, const FHhAxialCoord& Coord)
{
    UHhGridPanelSlotWidget* ChildSlot = Cast<UHhGridPanelSlotWidget>(AddChild(Content));
    if (ChildSlot)
    {
        ChildSlot->SetCoord(Coord);
        if (SlateGridPanel.IsValid())
        {
            ChildSlot->BuildSlot(SlateGridPanel.ToSharedRef());
        }
    }
    return ChildSlot;
}
 
void UHhGridPanelWidget::OnSlotAdded(UPanelSlot* InSlot)
{
    if (SlateGridPanel.IsValid())
    {
        if (UHhGridPanelSlotWidget* ChildSlot = Cast<UHhGridPanelSlotWidget>(InSlot))
        {
            ChildSlot->BuildSlot(SlateGridPanel.ToSharedRef());
        }
    }
}
 
void UHhGridPanelWidget::OnSlotRemoved(UPanelSlot* InSlot)
{
    if (SlateGridPanel.IsValid())
    {
        if (UHhGridPanelSlotWidget* ChildSlot = Cast<UHhGridPanelSlotWidget>(InSlot))
        {
            SlateGridPanel->RemoveSlot(ChildSlot->GetCoord());
        }
    }
}
 
TSharedRef<SWidget> UHhGridPanelWidget::RebuildWidget()
{
    SlateGridPanel = SNew(SHhGridPanel)
        .HexSize(HexSize)
        .HorizontalSize(HorizontalSize)
        .VerticalSize(VerticalSize);
 
    for (UPanelSlot* PanelSlot : Slots)
    {
        if (UHhGridPanelSlotWidget* TypedSlot = Cast<UHhGridPanelSlotWidget>(PanelSlot))
        {
            TypedSlot->BuildSlot(SlateGridPanel.ToSharedRef());
        }
    }
 
    return SlateGridPanel.ToSharedRef();
}

void UHhGridPanelWidget::ReleaseSlateResources(bool bReleaseChildren)
{
    Super::ReleaseSlateResources(bReleaseChildren);
    if (bReleaseChildren)
    {
        SlateGridPanel->ClearChildren();
        SlateGridPanel.Reset();
    }
}
