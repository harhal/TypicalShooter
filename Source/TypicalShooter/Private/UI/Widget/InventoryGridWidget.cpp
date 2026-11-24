// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widget/InventoryGridWidget.h"

#include "UI/ItemsContainer/ItemsContainer_Presenter.h"
#include "UI/Slate/InventoryGrid.h"

void UInventoryGridWidget::AssignPresenter(UItemsContainer_Presenter* InPresenter)
{
	if (Presenter && Presenter != InPresenter)
	{
		Presenter->SetView(nullptr);
	}

	Presenter = InPresenter;
	if (Presenter && SlateGrid.IsValid())
	{
		Presenter->SetView(SlateGrid);
	}
}

int32 UInventoryGridWidget::GetNumDisplayedItems() const
{
	return SlateGrid.IsValid() ? SlateGrid->GetNumItemViews() : 0;
}

void UInventoryGridWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	SlateGrid.Reset();
}

TSharedRef<SWidget> UInventoryGridWidget::RebuildWidget()
{
	SlateGrid = SNew(SInventoryGrid).HexSize(HexSize).GridSize(PreviewGridSize);
	if (Presenter)
	{
		Presenter->SetView(SlateGrid);
	}
	return SlateGrid.ToSharedRef();
}
