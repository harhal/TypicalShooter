// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HhGridPanel.h"
#include "UI/ItemsContainer/ItemsContainer_View.h"
#include "Widgets/SCompoundWidget.h"

class SInventoryGridItem;

/**
 * Hex grid of inventory items (view side of UItemsContainer_Presenter).
 */
class TYPICALSHOOTER_API SInventoryGrid : public SCompoundWidget, public IItemsContainer_View
{
public:
	SLATE_BEGIN_ARGS(SInventoryGrid)
		: _HexSize(50.f)
		, _GridSize(3)
	{ }
		SLATE_ARGUMENT(float, HexSize)
		SLATE_ARGUMENT(int32, GridSize)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	virtual FGuid AddInventoryItem(UInventoryItem_Presenter* Item, const FHhAxialCoord& Location) override;
	virtual void MoveInventoryItem(const FGuid& SlotGuid, const FHhAxialCoord& Location) override;
	virtual void RemoveInventoryItem(const FGuid& SlotGuid) override;
	virtual void ClearInventoryItems() override;

private:
	TSharedPtr<SHhGridPanel> GridPanel;

	/** Slots are owned by the panel; items are tracked by their widget. */
	TMap<FGuid, TSharedRef<SInventoryGridItem>> ItemWidgets;
};
