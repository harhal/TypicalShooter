// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Slate/InventoryGrid.h"

#include "HhGridFunctions.h"
#include "HhGridPanel.h"
#include "SlateOptMacros.h"
#include "Styling/CoreStyle.h"
#include "UI/InventoryItem/InventoryItem_Presenter.h"
#include "UI/ItemsContainer/ItemsContainer_Presenter.h"
#include "UI/Slate/InventoryGridItem.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"

// ---------------------------------------------------------------------------------------------------------------------
// FInventoryItemDragDropOp

TSharedRef<FInventoryItemDragDropOp> FInventoryItemDragDropOp::New(const FInventoryItemDragPayload& InPayload, const FSlateBrush& InIcon, const FVector2D& InIconSize)
{
	TSharedRef<FInventoryItemDragDropOp> Op = MakeShared<FInventoryItemDragDropOp>();
	Op->Payload = InPayload;
	Op->Icon = InIcon;
	Op->IconSize = InIconSize;
	Op->Construct();
	return Op;
}

TSharedPtr<SWidget> FInventoryItemDragDropOp::GetDefaultDecorator() const
{
	return SNew(SBox)
		.WidthOverride(IconSize.X)
		.HeightOverride(IconSize.Y)
		[
			SNew(SImage)
			.Image(&Icon)
			.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.8f))
		];
}

// ---------------------------------------------------------------------------------------------------------------------
// SInventoryGrid

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SInventoryGrid::Construct(const FArguments& InArgs)
{
	HexSize = InArgs._HexSize > 0.f ? InArgs._HexSize : 50.f;
	HighlightBrush = FSlateRoundedBoxBrush(FLinearColor::White, 0.75f * HexSize);
	CellBrush = InArgs._CellBrush;
	CellColor = InArgs._CellColor;
	HoveredCellColor = InArgs._HoveredCellColor;
	SelectedCellColor = InArgs._SelectedCellColor;
	DropTargetCellColor = InArgs._DropTargetCellColor;
	CursorCellColor = InArgs._CursorCellColor;

	ChildSlot
	[
		SNew(SOverlay)
		.Visibility(EVisibility::HitTestInvisible)

		+ SOverlay::Slot()
		[
			SAssignNew(CellsPanel, SHhGridPanel)
			.HexSize(HexSize)
		]

		+ SOverlay::Slot()
		[
			SAssignNew(ItemsPanel, SHhGridPanel)
			.HexSize(HexSize)
		]
	];

	SetGridRadius(FMath::Max(0, InArgs._GridRadius));
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SInventoryGrid::SetPresenter(UItemsContainer_Presenter* InPresenter)
{
	Presenter = InPresenter;
}

void SInventoryGrid::SetGridRadius(const int32 Radius)
{
	GridRadius = FMath::Max(0, Radius);
	const int32 Diameter = 2 * GridRadius + 1;
	CellsPanel->SetGridSize(Diameter, Diameter);
	ItemsPanel->SetGridSize(Diameter, Diameter);
	RebuildCells();
}

void SInventoryGrid::RebuildCells()
{
	CellsPanel->ClearChildren();
	NumCells = UHhGridFunctions::GetHexSpiralLength(GridRadius);

	const FSlateBrush* Brush = CellBrush ? CellBrush : FCoreStyle::Get().GetBrush("WhiteBrush");
	for (int32 N = 0; N < NumCells; ++N)
	{
		const FHhAxialCoord Coord = UHhGridFunctions::MakeHexAxialCoordFromSpiral(N);
		// Cell image (usually a hex outline) plus a round fill behind the item that shows hover / selection / drop target.
		CellsPanel->AddSlot(Coord)
		[
			SNew(SOverlay)

			+ SOverlay::Slot()
			.Padding(FMargin(0.12f * HexSize, 0.22f * HexSize))
			[
				SNew(SImage)
				.Image(&HighlightBrush)
				.ColorAndOpacity(TAttribute<FSlateColor>::CreateSP(this, &SInventoryGrid::GetHighlightColor, Coord))
			]

			+ SOverlay::Slot()
			[
				SNew(SImage)
				.Image(Brush)
				.ColorAndOpacity(CellColor)
			]
		];
	}
}

bool SInventoryGrid::IsInGrid(const FHhAxialCoord& Location) const
{
	return UHhGridFunctions::GetHexDistance(Location, FHhAxialCoord(0, 0)) <= GridRadius;
}

FGuid SInventoryGrid::AddInventoryItem(UInventoryItem_Presenter* Item, const FHhAxialCoord& Location)
{
	const TSharedRef<SInventoryGridItem> ItemWidget = SNew(SInventoryGridItem);
	ItemsPanel->AddSlot(Location)
	[
		ItemWidget
	];

	const FGuid SlotGuid = FGuid::NewGuid();
	ItemWidgets.Add(SlotGuid, ItemWidget);

	if (Item)
	{
		Item->SetView(ItemWidget);
	}

	return SlotGuid;
}

void SInventoryGrid::MoveInventoryItem(const FGuid& SlotGuid, const FHhAxialCoord& Location)
{
	const TSharedRef<SInventoryGridItem>* ItemWidget = ItemWidgets.Find(SlotGuid);
	if (!ItemWidget)
	{
		return;
	}

	if (SHhGridPanel::FSlot* Slot = ItemsPanel->FindSlot(*ItemWidget))
	{
		Slot->SetCoord(Location);
	}
}

void SInventoryGrid::RemoveInventoryItem(const FGuid& SlotGuid)
{
	TSharedPtr<SInventoryGridItem> ItemWidget;
	if (TSharedRef<SInventoryGridItem>* Found = ItemWidgets.Find(SlotGuid))
	{
		ItemWidget = *Found;
		ItemWidgets.Remove(SlotGuid);
	}

	if (ItemWidget.IsValid())
	{
		ItemsPanel->RemoveSlot(ItemWidget.ToSharedRef());
	}

	if (HoveredSlot == SlotGuid)
	{
		HoveredSlot.Invalidate();
	}
	if (SelectedSlot == SlotGuid)
	{
		SelectedSlot.Invalidate();
	}
}

void SInventoryGrid::ClearInventoryItems()
{
	ItemWidgets.Reset();
	ItemsPanel->ClearChildren();
	HoveredSlot.Invalidate();
	SelectedSlot.Invalidate();
}

TOptional<FHhAxialCoord> SInventoryGrid::GetSlotLocation(const FGuid& SlotGuid) const
{
	const TSharedRef<SInventoryGridItem>* ItemWidget = ItemWidgets.Find(SlotGuid);
	if (!ItemWidget)
	{
		return {};
	}
	const SHhGridPanel::FSlot* Slot = ItemsPanel->FindSlot(*ItemWidget);
	return Slot ? TOptional<FHhAxialCoord>(Slot->GetCoord()) : TOptional<FHhAxialCoord>();
}

FGuid SInventoryGrid::GetSlotAt(const FHhAxialCoord& Location) const
{
	for (const TPair<FGuid, TSharedRef<SInventoryGridItem>>& Pair : ItemWidgets)
	{
		if (GetSlotLocation(Pair.Key) == TOptional<FHhAxialCoord>(Location))
		{
			return Pair.Key;
		}
	}
	return FGuid();
}

FHhAxialCoord SInventoryGrid::GetCoordAtLocalPosition(const FVector2D& LocalPosition, const FVector2D& LocalSize) const
{
	return UHhGridFunctions::PointToHexAxialCoord(LocalPosition - LocalSize * 0.5f, HexSize);
}

FHhAxialCoord SInventoryGrid::GetCoordUnderCursor(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const
{
	return GetCoordAtLocalPosition(MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()), MyGeometry.GetLocalSize());
}

TOptional<FInventoryItemDragPayload> SInventoryGrid::MakeDragPayload(const FHhAxialCoord& Location) const
{
	const FGuid SlotGuid = GetSlotAt(Location);
	if (!SlotGuid.IsValid() || !Presenter.IsValid())
	{
		return {};
	}

	FInventoryItemDragPayload Payload;
	Payload.SourcePresenter = Presenter;
	Payload.SourceSlot = SlotGuid;
	Payload.SourceLocation = Location;
	return Payload;
}

bool SInventoryGrid::DropPayload(const FInventoryItemDragPayload& Payload, const FHhAxialCoord& Location)
{
	UItemsContainer_Presenter* TargetPresenter = Presenter.Get();
	if (!TargetPresenter || !IsInGrid(Location))
	{
		return false;
	}
	if (Payload.SourcePresenter.Get() == TargetPresenter && Payload.SourceLocation == Location)
	{
		return false;
	}
	return TargetPresenter->HandleItemDropped(Payload.SourcePresenter.Get(), Payload.SourceSlot, Location);
}

void SInventoryGrid::SetHoveredCell(const TOptional<FHhAxialCoord>& Location)
{
	HoveredCell = Location.IsSet() && IsInGrid(Location.GetValue()) ? Location : TOptional<FHhAxialCoord>();

	const FGuid NewHoveredSlot = HoveredCell.IsSet() ? GetSlotAt(HoveredCell.GetValue()) : FGuid();
	if (NewHoveredSlot == HoveredSlot)
	{
		return;
	}
	HoveredSlot = NewHoveredSlot;
	if (UItemsContainer_Presenter* PinnedPresenter = Presenter.Get())
	{
		PinnedPresenter->HandleItemHovered(HoveredSlot);
	}
}

void SInventoryGrid::ClickCell(const FHhAxialCoord& Location)
{
	if (UItemsContainer_Presenter* PinnedPresenter = Presenter.Get())
	{
		PinnedPresenter->HandleItemClicked(GetSlotAt(Location));
	}
}

void SInventoryGrid::SetDropTargetCell(const TOptional<FHhAxialCoord>& Location)
{
	DropTargetCell = Location.IsSet() && IsInGrid(Location.GetValue()) ? Location : TOptional<FHhAxialCoord>();
}

void SInventoryGrid::SetSelectedSlot(const FGuid& SlotGuid)
{
	SelectedSlot = ItemWidgets.Contains(SlotGuid) ? SlotGuid : FGuid();
}

void SInventoryGrid::SetCursorCell(const TOptional<FHhAxialCoord>& Location)
{
	CursorCell = Location.IsSet() && IsInGrid(Location.GetValue()) ? Location : TOptional<FHhAxialCoord>();
}

EInventoryCellHighlight SInventoryGrid::GetCellHighlight(const FHhAxialCoord& Location) const
{
	if (DropTargetCell == TOptional<FHhAxialCoord>(Location))
	{
		return EInventoryCellHighlight::DropTarget;
	}
	if (SelectedSlot.IsValid() && GetSlotLocation(SelectedSlot) == TOptional<FHhAxialCoord>(Location))
	{
		return EInventoryCellHighlight::Selected;
	}
	if (CursorCell == TOptional<FHhAxialCoord>(Location))
	{
		return EInventoryCellHighlight::Cursor;
	}
	if (HoveredCell == TOptional<FHhAxialCoord>(Location))
	{
		return EInventoryCellHighlight::Hovered;
	}
	return EInventoryCellHighlight::None;
}

FSlateColor SInventoryGrid::GetHighlightColor(const FHhAxialCoord Location) const
{
	switch (GetCellHighlight(Location))
	{
	case EInventoryCellHighlight::DropTarget:
		return DropTargetCellColor;
	case EInventoryCellHighlight::Selected:
		return SelectedCellColor;
	case EInventoryCellHighlight::Cursor:
		return CursorCellColor;
	case EInventoryCellHighlight::Hovered:
		return HoveredCellColor;
	default:
		return FLinearColor::Transparent;
	}
}

FReply SInventoryGrid::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return FReply::Unhandled();
	}

	const FHhAxialCoord Location = GetCoordUnderCursor(MyGeometry, MouseEvent);
	if (!IsInGrid(Location))
	{
		return FReply::Unhandled();
	}

	PressedCell = Location;
	FReply Reply = FReply::Handled();
	if (GetSlotAt(Location).IsValid())
	{
		Reply.DetectDrag(SharedThis(this), EKeys::LeftMouseButton);
	}
	return Reply;
}

FReply SInventoryGrid::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton || !PressedCell.IsSet())
	{
		return FReply::Unhandled();
	}

	const FHhAxialCoord Location = GetCoordUnderCursor(MyGeometry, MouseEvent);
	if (PressedCell.GetValue() == Location)
	{
		ClickCell(Location);
	}
	PressedCell.Reset();
	return FReply::Handled();
}

FReply SInventoryGrid::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	SetHoveredCell(GetCoordUnderCursor(MyGeometry, MouseEvent));
	return FReply::Unhandled();
}

void SInventoryGrid::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	SCompoundWidget::OnMouseLeave(MouseEvent);
	SetHoveredCell({});
	PressedCell.Reset();
}

FReply SInventoryGrid::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (!PressedCell.IsSet())
	{
		return FReply::Unhandled();
	}

	const TOptional<FInventoryItemDragPayload> Payload = MakeDragPayload(PressedCell.GetValue());
	PressedCell.Reset();
	if (!Payload.IsSet())
	{
		return FReply::Unhandled();
	}

	FSlateBrush Icon;
	if (const TSharedRef<SInventoryGridItem>* ItemWidget = ItemWidgets.Find(Payload->SourceSlot))
	{
		Icon = (*ItemWidget)->GetIconBrush();
	}
	return FReply::Handled().BeginDragDrop(FInventoryItemDragDropOp::New(Payload.GetValue(), Icon, FVector2D(HexSize)));
}

void SInventoryGrid::OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	if (DragDropEvent.GetOperationAs<FInventoryItemDragDropOp>().IsValid())
	{
		SetDropTargetCell(GetCoordUnderCursor(MyGeometry, DragDropEvent));
	}
}

void SInventoryGrid::OnDragLeave(const FDragDropEvent& DragDropEvent)
{
	SetDropTargetCell({});
}

FReply SInventoryGrid::OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	if (!DragDropEvent.GetOperationAs<FInventoryItemDragDropOp>().IsValid())
	{
		return FReply::Unhandled();
	}
	SetDropTargetCell(GetCoordUnderCursor(MyGeometry, DragDropEvent));
	return FReply::Handled();
}

FReply SInventoryGrid::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	SetDropTargetCell({});

	const TSharedPtr<FInventoryItemDragDropOp> Op = DragDropEvent.GetOperationAs<FInventoryItemDragDropOp>();
	if (!Op.IsValid())
	{
		return FReply::Unhandled();
	}
	DropPayload(Op->Payload, GetCoordUnderCursor(MyGeometry, DragDropEvent));
	return FReply::Handled();
}
