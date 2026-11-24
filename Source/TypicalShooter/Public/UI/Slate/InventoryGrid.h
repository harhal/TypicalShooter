// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "HhAxialCoord.h"
#include "Input/DragAndDrop.h"
#include "UI/ItemsContainer/ItemsContainer_View.h"
#include "Widgets/SCompoundWidget.h"

class SHhGridPanel;
class SInventoryGridItem;
class UItemsContainer_Presenter;

/** What is being dragged: an item slot of some grid's presenter. */
struct FInventoryItemDragPayload
{
	TWeakObjectPtr<UItemsContainer_Presenter> SourcePresenter;
	FGuid SourceSlot;
	FHhAxialCoord SourceLocation;
};

/** Drag & drop operation carrying an inventory item between SInventoryGrids. */
class TYPICALSHOOTER_API FInventoryItemDragDropOp : public FDragDropOperation
{
public:
	DRAG_DROP_OPERATOR_TYPE(FInventoryItemDragDropOp, FDragDropOperation)

	static TSharedRef<FInventoryItemDragDropOp> New(const FInventoryItemDragPayload& InPayload, const FSlateBrush& InIcon, const FVector2D& InIconSize);

	virtual TSharedPtr<SWidget> GetDefaultDecorator() const override;

	FInventoryItemDragPayload Payload;

private:
	FSlateBrush Icon;
	FVector2D IconSize = FVector2D(64.f);
};

enum class EInventoryCellHighlight : uint8
{
	None,
	Hovered,
	Selected,
	DropTarget,
};

/**
 * Hex grid of inventory items (view side of UItemsContainer_Presenter): background cells for the whole grid radius,
 * item views on top. Handles hover, click and drag & drop itself and reports them to the presenter.
 */
class TYPICALSHOOTER_API SInventoryGrid : public SCompoundWidget, public IItemsContainer_View
{
public:
	SLATE_BEGIN_ARGS(SInventoryGrid)
		: _HexSize(50.f)
		, _GridRadius(1)
		, _CellBrush(nullptr)
		, _CellColor(FLinearColor(1.f, 1.f, 1.f, 0.25f))
		, _HoveredCellColor(FLinearColor(1.f, 1.f, 1.f, 0.3f))
		, _SelectedCellColor(FLinearColor(1.f, 0.75f, 0.2f, 0.5f))
		, _DropTargetCellColor(FLinearColor(0.3f, 1.f, 0.4f, 0.5f))
	{ }
		SLATE_ARGUMENT(float, HexSize)
		/** Radius shown until a presenter sets the model's one. */
		SLATE_ARGUMENT(int32, GridRadius)
		/** Brush of a background cell (a hex image / material); a plain box if null. Must outlive the widget. */
		SLATE_ARGUMENT(const FSlateBrush*, CellBrush)
		SLATE_ARGUMENT(FLinearColor, CellColor)
		SLATE_ARGUMENT(FLinearColor, HoveredCellColor)
		SLATE_ARGUMENT(FLinearColor, SelectedCellColor)
		SLATE_ARGUMENT(FLinearColor, DropTargetCellColor)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	// IItemsContainer_View
	virtual void SetPresenter(UItemsContainer_Presenter* InPresenter) override;
	virtual void SetGridRadius(int32 Radius) override;
	virtual FGuid AddInventoryItem(UInventoryItem_Presenter* Item, const FHhAxialCoord& Location) override;
	virtual void MoveInventoryItem(const FGuid& SlotGuid, const FHhAxialCoord& Location) override;
	virtual void RemoveInventoryItem(const FGuid& SlotGuid) override;
	virtual void ClearInventoryItems() override;

	int32 GetGridRadius() const { return GridRadius; }
	int32 GetNumCells() const { return NumCells; }
	int32 GetNumItemViews() const { return ItemWidgets.Num(); }
	bool IsInGrid(const FHhAxialCoord& Location) const;

	/** Slot of the item view at Location, invalid if the cell is empty. */
	FGuid GetSlotAt(const FHhAxialCoord& Location) const;

	/** Cell under a point in this widget's local space (the grid is centred in LocalSize). */
	FHhAxialCoord GetCoordAtLocalPosition(const FVector2D& LocalPosition, const FVector2D& LocalSize) const;

	// Input, also driven directly by tests.
	TOptional<FInventoryItemDragPayload> MakeDragPayload(const FHhAxialCoord& Location) const;
	/** Drops a dragged item at Location of this grid. Returns true if the presenter requested a transfer. */
	bool DropPayload(const FInventoryItemDragPayload& Payload, const FHhAxialCoord& Location);
	/** Unset = the cursor left the grid. */
	void SetHoveredCell(const TOptional<FHhAxialCoord>& Location);
	void ClickCell(const FHhAxialCoord& Location);
	void SetDropTargetCell(const TOptional<FHhAxialCoord>& Location);

	/** Item view drawn as selected (invalid guid = none). */
	void SetSelectedSlot(const FGuid& SlotGuid);

	EInventoryCellHighlight GetCellHighlight(const FHhAxialCoord& Location) const;

	// SWidget
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual void OnDragLeave(const FDragDropEvent& DragDropEvent) override;
	virtual FReply OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;

private:
	void RebuildCells();
	FSlateColor GetHighlightColor(FHhAxialCoord Location) const;
	FHhAxialCoord GetCoordUnderCursor(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) const;
	TOptional<FHhAxialCoord> GetSlotLocation(const FGuid& SlotGuid) const;

	TSharedPtr<SHhGridPanel> CellsPanel;
	TSharedPtr<SHhGridPanel> ItemsPanel;

	/** Slots are owned by the panel; items are tracked by their widget. */
	TMap<FGuid, TSharedRef<SInventoryGridItem>> ItemWidgets;

	TWeakObjectPtr<UItemsContainer_Presenter> Presenter;

	float HexSize = 50.f;
	int32 GridRadius = 1;
	int32 NumCells = 0;

	const FSlateBrush* CellBrush = nullptr;
	/** Round fill drawn behind items for hover / selection / drop target. */
	FSlateRoundedBoxBrush HighlightBrush = FSlateRoundedBoxBrush(FLinearColor::White, 1.f);
	FLinearColor CellColor;
	FLinearColor HoveredCellColor;
	FLinearColor SelectedCellColor;
	FLinearColor DropTargetCellColor;

	TOptional<FHhAxialCoord> HoveredCell;
	FGuid HoveredSlot;
	FGuid SelectedSlot;
	TOptional<FHhAxialCoord> DropTargetCell;
	/** Cell the left mouse button went down on (click / drag source). */
	TOptional<FHhAxialCoord> PressedCell;
};
