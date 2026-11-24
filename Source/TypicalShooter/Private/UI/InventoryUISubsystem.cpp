// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/InventoryUISubsystem.h"

#include "InventoryItem.h"

void UInventoryUISubsystem::SetHoveredItem(UInventoryItem* Item)
{
	if (HoveredItem.Get() == Item)
	{
		return;
	}
	HoveredItem = Item;
	NotifyChanged();
}

void UInventoryUISubsystem::SelectItem(UInventoryItem* Item)
{
	UInventoryItem* NewSelection = SelectedItem.Get() == Item ? nullptr : Item;
	if (SelectedItem.Get() == NewSelection)
	{
		return;
	}
	SelectedItem = NewSelection;
	NotifyChanged();
}

void UInventoryUISubsystem::ForgetItem(const UInventoryItem* Item)
{
	if (!Item)
	{
		return;
	}

	bool bChanged = false;
	if (HoveredItem.Get() == Item)
	{
		HoveredItem.Reset();
		bChanged = true;
	}
	if (SelectedItem.Get() == Item)
	{
		SelectedItem.Reset();
		bChanged = true;
	}
	if (bChanged)
	{
		NotifyChanged();
	}
}

UInventoryItem* UInventoryUISubsystem::GetDisplayedItem() const
{
	UInventoryItem* Hovered = HoveredItem.Get();
	return Hovered ? Hovered : SelectedItem.Get();
}

void UInventoryUISubsystem::NotifyChanged()
{
	OnSelectionChanged.Broadcast();
	OnDisplayedItemChanged.Broadcast(GetDisplayedItem());
}
