// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widget/InventoryGridWidget.h"
#include "UI/ItemsContainer/ItemsContainer_Presenter.h"

void UInventoryGridWidget::AssignPresenter(UItemsContainer_Presenter* InPresenter)
{
	Presenter = TStrongObjectPtr(InPresenter);
	Presenter->SetView(SlateGrid);
}

void UInventoryGridWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	SlateGrid.Reset();
}

TSharedRef<SWidget> UInventoryGridWidget::RebuildWidget()
{
	SlateGrid = SNew(SInventoryGrid).HexSize(HexSize).GridSize(PreviewGridSize);
	if (Presenter.IsValid())
	{
		Presenter->SetView(SlateGrid);
	}
	return SlateGrid.ToSharedRef();
}

