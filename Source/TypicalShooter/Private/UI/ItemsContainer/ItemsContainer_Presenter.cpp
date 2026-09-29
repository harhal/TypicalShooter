// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/ItemsContainer/ItemsContainer_Presenter.h"

#include "InventoryItem.h"
#include "UI/InventoryItem/InventoryItem_Presenter.h"
#include "UI/ItemsContainer/ItemsContainer_Model.h"
#include "UI/ItemsContainer/ItemsContainer_View.h"

void UItemsContainer_Presenter::SetModel(const TScriptInterface<IItemsContainer_Model>& InModel)
{
	Model = InModel;
	InModel->GetItemAddedDelegate().AddUObject(this, &UItemsContainer_Presenter::OnItemAdded);
	InModel->GetItemRemovedDelegate().AddUObject(this, &UItemsContainer_Presenter::OnItemRemoved);
	InModel->GetItemMovedDelegate().AddUObject(this, &UItemsContainer_Presenter::OnItemMoved);
	
	UpdateView();
}

void UItemsContainer_Presenter::SetView(const TSharedPtr<IItemsContainer_View>& InView)
{
	View = InView;
	InView->GetAddItemDelegate();
	InView->GetRemoveItemDelegate();
	InView->GetMoveItemDelegate();
	
	UpdateView();
}

void UItemsContainer_Presenter::OnItemAdded(const UInventoryItem* InventoryItem,
	const FHhAxialCoord& HhAxialCoord) const
{
}

void UItemsContainer_Presenter::OnItemRemoved(const UInventoryItem* InventoryItem) const
{
}

void UItemsContainer_Presenter::OnItemMoved(const UInventoryItem* InventoryItem, const FHhAxialCoord& HhAxialCoord,
	const FHhAxialCoord& HhAxialCoord1) const
{
}

void UItemsContainer_Presenter::UpdateView() const
{
	if (!View.Get())
	{
		return;
	}
	
	if (IsValid(Model.GetObject()))
	{
		for (TPair<UInventoryItem*, FHhAxialCoord>& PlacedItem : Model->GetPlacedItems())
		{
			UInventoryItem_Presenter* Presenter = NewObject<UInventoryItem_Presenter>();
			UInventoryItem* Item = PlacedItem.Key;
			if (!IsValid(Item))
			{
				continue;
			}

			Presenter->SetModel(Item);
			View->AddInventoryItem(Presenter, PlacedItem.Value);
		}
	}	
}
