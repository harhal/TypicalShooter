// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widget/InventoryItemInfoWidget.h"

#include "Engine/LocalPlayer.h"
#include "InventoryItem.h"
#include "UI/InventoryItem/InventoryItem_Presenter.h"
#include "UI/InventoryUISubsystem.h"
#include "UI/Slate/InventoryItemInfo.h"

void UInventoryItemInfoWidget::SetItem(UInventoryItem* Item)
{
	ShownItem = Item;
	if (Presenter)
	{
		Presenter->SetModel(Item);
	}
}

void UInventoryItemInfoWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	if (const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (UInventoryUISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UInventoryUISubsystem>())
		{
			UISubsystem->OnSelectionChanged.Remove(SelectionChangedHandle);
		}
	}
	SelectionChangedHandle.Reset();

	if (Presenter)
	{
		Presenter->SetView(nullptr);
	}
	SlateInfo.Reset();
}

TSharedRef<SWidget> UInventoryItemInfoWidget::RebuildWidget()
{
	SlateInfo = SNew(SInventoryItemInfo).IconSize(IconSize);

	if (!Presenter)
	{
		Presenter = NewObject<UInventoryItem_Presenter>(this);
	}
	Presenter->SetView(SlateInfo);

	const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	UInventoryUISubsystem* UISubsystem = LocalPlayer ? LocalPlayer->GetSubsystem<UInventoryUISubsystem>() : nullptr;
	if (bFollowInventorySelection && UISubsystem)
	{
		if (!SelectionChangedHandle.IsValid())
		{
			SelectionChangedHandle = UISubsystem->OnSelectionChanged.AddUObject(this, &UInventoryItemInfoWidget::HandleSelectionChanged);
		}
		SetItem(UISubsystem->GetDisplayedItem());
	}
	else
	{
		SetItem(ShownItem.Get());
	}

	return SlateInfo.ToSharedRef();
}

void UInventoryItemInfoWidget::HandleSelectionChanged()
{
	const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	const UInventoryUISubsystem* UISubsystem = LocalPlayer ? LocalPlayer->GetSubsystem<UInventoryUISubsystem>() : nullptr;
	SetItem(UISubsystem ? UISubsystem->GetDisplayedItem() : nullptr);
}
