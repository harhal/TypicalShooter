// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HhGridPanel.h"
#include "InventoryGridItem.h"
#include "UI/ItemsContainer/ItemsContainer_View.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class TYPICALSHOOTER_API SInventoryGrid : public SCompoundWidget, public IItemsContainer_View
{
public:
	SLATE_BEGIN_ARGS(SInventoryGrid) { }
		SLATE_ARGUMENT(float, HexSize)
		SLATE_ARGUMENT(int32, GridSize)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	
	virtual FGuid AddInventoryItem(UInventoryItem_Presenter* Item, const FHhAxialCoord& Location) override;
	virtual void MoveInventoryItem(const FGuid& SlotGuid, const FHhAxialCoord& Location) override;
	virtual void RemoveInventoryItem(const FGuid& SlotGuid) override;
	
	virtual void GetAddItemDelegate() override;
	virtual void GetRemoveItemDelegate() override;
	virtual void GetMoveItemDelegate() override;
 
private:
	TSharedRef<SHhGridPanel> GridPanel = SNew(SHhGridPanel);
	TMap<FGuid, TSharedRef<SHhGridPanel::FSlot>> ChildSlots;
	
	float HexSize = 50.f;
	int32 GridSize = 3;
};
