// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Slate/InventoryGrid.h"

#include "HhGridPanel.h"
#include "SlateOptMacros.h"
#include "UI/InventoryItem/InventoryItem_Presenter.h"
#include "UI/Slate/InventoryGridItem.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SInventoryGrid::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SAssignNew(GridPanel, SHhGridPanel)
		.HexSize(InArgs._HexSize)
		.HorizontalSize(InArgs._GridSize)
		.VerticalSize(InArgs._GridSize)
	];
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FGuid SInventoryGrid::AddInventoryItem(UInventoryItem_Presenter* Item, const FHhAxialCoord& Location)
{
	const TSharedRef<SInventoryGridItem> ItemWidget = SNew(SInventoryGridItem);
	GridPanel->AddSlot(Location)
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

	if (SHhGridPanel::FSlot* Slot = GridPanel->FindSlot(*ItemWidget))
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
		GridPanel->RemoveSlot(ItemWidget.ToSharedRef());
	}
}

void SInventoryGrid::ClearInventoryItems()
{
	ItemWidgets.Reset();
	GridPanel->ClearChildren();
}
