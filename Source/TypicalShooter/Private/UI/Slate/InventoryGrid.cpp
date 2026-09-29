// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Slate/InventoryGrid.h"

#include "HhGridPanel.h"
#include "SlateOptMacros.h"
#include "UI/InventoryItem/InventoryItem_Presenter.h"
#include "UI/Slate/InventoryGridItem.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SInventoryGrid::Construct(const FArguments& InArgs)
{
	GridPanel = SNew(SHhGridPanel)
		.HexSize(InArgs._HexSize)
		.HorizontalSize(InArgs._GridSize)
		.VerticalSize(InArgs._GridSize);
	
	ChildSlot
	[
		GridPanel
	];
}

FGuid SInventoryGrid::AddInventoryItem(UInventoryItem_Presenter* Item, const FHhAxialCoord& Location)
{
	const TSharedRef<SInventoryGridItem> InventoryGridItem = SNew(SInventoryGridItem);
	SHhGridPanel::FSlot* Slot;
	GridPanel->AddSlot(Location)
	.Expose(Slot)
	[
		InventoryGridItem
	];
	const FGuid& SlotGuid = FGuid::NewGuid();
	ChildSlots.Add(SlotGuid, MakeShareable(Slot));
	
	Item->SetView(InventoryGridItem);
	
	return SlotGuid;
}

void SInventoryGrid::MoveInventoryItem(const FGuid& SlotGuid, const FHhAxialCoord& Location)
{
	if (ChildSlots.Contains(SlotGuid))
	{
		ChildSlots[SlotGuid]->SetCoord(Location);
	}
}

void SInventoryGrid::RemoveInventoryItem(const FGuid& SlotGuid)
{
	if (ChildSlots.Contains(SlotGuid))
	{
		ChildSlots.Remove(SlotGuid);
	}
}

void SInventoryGrid::GetAddItemDelegate()
{
}

void SInventoryGrid::GetRemoveItemDelegate()
{
}

void SInventoryGrid::GetMoveItemDelegate()
{
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
