// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/ItemsContainer/ItemsContainer_Presenter.h"

#include "InventoryItem.h"
#include "UI/InventoryItem/InventoryItem_Presenter.h"
#include "UI/ItemsContainer/ItemsContainer_View.h"

UItemsContainer_Presenter* UItemsContainer_Presenter::CreatePresenter(const TScriptInterface<IItemsContainer_Model>& InModel)
{
	UItemsContainer_Presenter* Presenter = NewObject<UItemsContainer_Presenter>(GetTransientPackage());
	Presenter->SetModel(InModel);
	return Presenter;
}

void UItemsContainer_Presenter::BeginDestroy()
{
	UnbindModel();
	Super::BeginDestroy();
}

void UItemsContainer_Presenter::SetModel(const TScriptInterface<IItemsContainer_Model>& InModel)
{
	UnbindModel();

	Model = InModel;
	if (IsValid(Model.GetObject()))
	{
		Model->GetItemAddedDelegate().AddUObject(this, &UItemsContainer_Presenter::OnItemAdded);
		Model->GetItemRemovedDelegate().AddUObject(this, &UItemsContainer_Presenter::OnItemRemoved);
		Model->GetItemMovedDelegate().AddUObject(this, &UItemsContainer_Presenter::OnItemMoved);
	}

	RebuildView();
}

void UItemsContainer_Presenter::SetView(const TSharedPtr<IItemsContainer_View>& InView)
{
	View = InView;
	if (InView.IsValid())
	{
		InView->SetPresenter(this);
	}
	RebuildView();
}

UInventoryItem* UItemsContainer_Presenter::GetItemInSlot(const FGuid& SlotGuid) const
{
	const TWeakObjectPtr<UInventoryItem>* Item = ItemsBySlot.Find(SlotGuid);
	return Item ? Item->Get() : nullptr;
}

FGuid UItemsContainer_Presenter::GetSlotOfItem(const UInventoryItem* Item) const
{
	const FGuid* SlotGuid = Item ? SlotsByItem.Find(Item) : nullptr;
	return SlotGuid ? *SlotGuid : FGuid();
}

bool UItemsContainer_Presenter::HandleItemDropped(UItemsContainer_Presenter* SourcePresenter, const FGuid& SourceSlot, const FHhAxialCoord& Location)
{
	if (!SourcePresenter || !IsValid(Model.GetObject()))
	{
		return false;
	}

	UInventoryItem* Item = SourcePresenter->GetItemInSlot(SourceSlot);
	const TScriptInterface<IItemsContainer_Model>& SourceModel = SourcePresenter->GetModel();
	if (!Item || !IsValid(SourceModel.GetObject()))
	{
		return false;
	}

	SourceModel->RequestTransferItem(Item, Model, Location);
	return true;
}

void UItemsContainer_Presenter::HandleItemHovered(const FGuid& SlotGuid)
{
	OnItemHovered.Broadcast(GetItemInSlot(SlotGuid));
}

void UItemsContainer_Presenter::HandleItemClicked(const FGuid& SlotGuid)
{
	OnItemClicked.Broadcast(GetItemInSlot(SlotGuid));
}

void UItemsContainer_Presenter::UnbindModel()
{
	if (IsValid(Model.GetObject()))
	{
		Model->GetItemAddedDelegate().RemoveAll(this);
		Model->GetItemRemovedDelegate().RemoveAll(this);
		Model->GetItemMovedDelegate().RemoveAll(this);
	}
	Model = nullptr;
}

void UItemsContainer_Presenter::RebuildView()
{
	PresentersBySlot.Reset();
	SlotsByItem.Reset();
	ItemsBySlot.Reset();

	const TSharedPtr<IItemsContainer_View> PinnedView = View.Pin();
	if (!PinnedView.IsValid())
	{
		return;
	}

	PinnedView->ClearInventoryItems();

	if (!IsValid(Model.GetObject()))
	{
		return;
	}

	PinnedView->SetGridRadius(Model->GetGridRadius());

	for (const TPair<UInventoryItem*, FHhAxialCoord>& PlacedItem : Model->GetPlacedItems())
	{
		AddItemToView(PlacedItem.Key, PlacedItem.Value);
	}
}

void UItemsContainer_Presenter::AddItemToView(const UInventoryItem* Item, const FHhAxialCoord& Location)
{
	const TSharedPtr<IItemsContainer_View> PinnedView = View.Pin();
	if (!PinnedView.IsValid() || !IsValid(Item) || SlotsByItem.Contains(Item))
	{
		return;
	}

	// Model delegates hand out const items; the presenter only reads from them and passes them back to the model.
	UInventoryItem* MutableItem = const_cast<UInventoryItem*>(Item);

	UInventoryItem_Presenter* ItemPresenter = NewObject<UInventoryItem_Presenter>(this);
	ItemPresenter->SetModel(MutableItem);

	const FGuid SlotGuid = PinnedView->AddInventoryItem(ItemPresenter, Location);
	PresentersBySlot.Add(SlotGuid, ItemPresenter);
	SlotsByItem.Add(Item, SlotGuid);
	ItemsBySlot.Add(SlotGuid, MutableItem);
}

void UItemsContainer_Presenter::OnItemAdded(const UInventoryItem* Item, const FHhAxialCoord& Location)
{
	AddItemToView(Item, Location);
}

void UItemsContainer_Presenter::OnItemRemoved(const UInventoryItem* Item)
{
	FGuid SlotGuid;
	if (!SlotsByItem.RemoveAndCopyValue(Item, SlotGuid))
	{
		return;
	}

	PresentersBySlot.Remove(SlotGuid);
	ItemsBySlot.Remove(SlotGuid);

	if (const TSharedPtr<IItemsContainer_View> PinnedView = View.Pin())
	{
		PinnedView->RemoveInventoryItem(SlotGuid);
	}

	OnItemViewRemoved.Broadcast(const_cast<UInventoryItem*>(Item));
}

void UItemsContainer_Presenter::OnItemMoved(const UInventoryItem* Item, const FHhAxialCoord& Location, const FHhAxialCoord& PrevLocation)
{
	const FGuid* SlotGuid = SlotsByItem.Find(Item);
	if (!SlotGuid)
	{
		return;
	}

	if (const TSharedPtr<IItemsContainer_View> PinnedView = View.Pin())
	{
		PinnedView->MoveInventoryItem(*SlotGuid, Location);
	}
}
