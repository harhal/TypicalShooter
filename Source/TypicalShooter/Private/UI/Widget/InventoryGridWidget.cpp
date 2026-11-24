// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widget/InventoryGridWidget.h"

#include "Engine/LocalPlayer.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "InventoryComponent.h"
#include "InventoryItem.h"
#include "TypicalShooter.h"
#include "UI/InventoryUISubsystem.h"
#include "UI/ItemsContainer/ItemsContainer_Presenter.h"
#include "UI/Slate/InventoryGrid.h"

void UInventoryGridWidget::AssignPresenter(UItemsContainer_Presenter* InPresenter)
{
	if (Presenter == InPresenter)
	{
		return;
	}

	UnbindPresenterEvents();
	if (Presenter)
	{
		Presenter->SetView(nullptr);
	}

	Presenter = InPresenter;
	if (Presenter && SlateGrid.IsValid())
	{
		Presenter->SetView(SlateGrid);
	}
	BindPresenterEvents();
	SyncSelection();
}

int32 UInventoryGridWidget::GetNumDisplayedItems() const
{
	return SlateGrid.IsValid() ? SlateGrid->GetNumItemViews() : 0;
}

void UInventoryGridWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	if (UInventoryUISubsystem* UISubsystem = GetUISubsystem())
	{
		UISubsystem->OnSelectionChanged.Remove(SelectionChangedHandle);
	}
	SelectionChangedHandle.Reset();

	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->OnPossessedPawnChanged.RemoveDynamic(this, &UInventoryGridWidget::HandlePossessedPawnChanged);
	}

	if (Presenter)
	{
		Presenter->SetView(nullptr);
	}
	SlateGrid.Reset();
}

TSharedRef<SWidget> UInventoryGridWidget::RebuildWidget()
{
	SlateGrid = SNew(SInventoryGrid)
		.HexSize(HexSize)
		.GridRadius(PreviewGridRadius)
		.CellBrush(CellBrush.GetResourceObject() ? &CellBrush : nullptr)
		.CellColor(CellColor)
		.HoveredCellColor(HoveredCellColor)
		.SelectedCellColor(SelectedCellColor)
		.DropTargetCellColor(DropTargetCellColor)
		.CursorCellColor(CursorCellColor);

	if (UInventoryUISubsystem* UISubsystem = GetUISubsystem(); UISubsystem && !SelectionChangedHandle.IsValid())
	{
		SelectionChangedHandle = UISubsystem->OnSelectionChanged.AddUObject(this, &UInventoryGridWidget::SyncSelection);
	}

	if (bShowOwningPawnInventory)
	{
		BindOwningPawnInventory();
	}

	if (Presenter)
	{
		Presenter->SetView(SlateGrid);
	}
	SyncSelection();

	return SlateGrid.ToSharedRef();
}

void UInventoryGridWidget::BindOwningPawnInventory()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		UE_LOG(LogTypicalShooter, Verbose, TEXT("%s: no owning player, cannot follow the pawn inventory"), *GetName());
		return;
	}
	PC->OnPossessedPawnChanged.AddUniqueDynamic(this, &UInventoryGridWidget::HandlePossessedPawnChanged);
	ShowPawnInventory(PC->GetPawn());
}

void UInventoryGridWidget::HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	ShowPawnInventory(NewPawn);
}

void UInventoryGridWidget::ShowPawnInventory(APawn* Pawn)
{
	UInventoryComponent* Inventory = Pawn ? Pawn->FindComponentByClass<UInventoryComponent>() : nullptr;
	UE_LOG(LogTypicalShooter, Verbose, TEXT("%s: showing inventory of pawn %s (%s)"), *GetName(), *GetNameSafe(Pawn), *GetNameSafe(Inventory));
	if (!Inventory)
	{
		AssignPresenter(nullptr);
		return;
	}
	if (Presenter && Presenter->GetModel().GetObject() == Inventory)
	{
		return;
	}
	AssignPresenter(UItemsContainer_Presenter::CreatePresenter(Inventory));
}

UInventoryUISubsystem* UInventoryGridWidget::GetUISubsystem() const
{
	const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	return LocalPlayer ? LocalPlayer->GetSubsystem<UInventoryUISubsystem>() : nullptr;
}

void UInventoryGridWidget::BindPresenterEvents()
{
	if (!Presenter)
	{
		return;
	}
	Presenter->OnItemHovered.AddUObject(this, &UInventoryGridWidget::HandleItemHovered);
	Presenter->OnItemClicked.AddUObject(this, &UInventoryGridWidget::HandleItemClicked);
	Presenter->OnItemViewRemoved.AddUObject(this, &UInventoryGridWidget::HandleItemViewRemoved);
}

void UInventoryGridWidget::UnbindPresenterEvents()
{
	if (!Presenter)
	{
		return;
	}
	Presenter->OnItemHovered.RemoveAll(this);
	Presenter->OnItemClicked.RemoveAll(this);
	Presenter->OnItemViewRemoved.RemoveAll(this);
}

void UInventoryGridWidget::SyncSelection()
{
	if (!SlateGrid.IsValid())
	{
		return;
	}
	const UInventoryUISubsystem* UISubsystem = GetUISubsystem();
	const UInventoryItem* Selected = UISubsystem ? UISubsystem->GetSelectedItem() : nullptr;
	SlateGrid->SetSelectedSlot(Presenter && Selected ? Presenter->GetSlotOfItem(Selected) : FGuid());

	// The cursor container may be given as the component or its actor (same for the presenter's model).
	const UInventoryComponent* GridInventory = Presenter ? UInventoryComponent::FindInventory(Presenter->GetModel().GetObject()) : nullptr;
	const bool bCursorHere = UISubsystem && UISubsystem->IsCursorActive() && GridInventory
		&& UInventoryComponent::FindInventory(UISubsystem->GetCursorContainer()) == GridInventory;
	SlateGrid->SetCursorCell(bCursorHere ? TOptional<FHhAxialCoord>(UISubsystem->GetCursorLocation()) : TOptional<FHhAxialCoord>());
}

void UInventoryGridWidget::HandleItemHovered(UInventoryItem* Item)
{
	if (UInventoryUISubsystem* UISubsystem = GetUISubsystem())
	{
		// Hover changes only come from the mouse moving: it takes over from the gamepad / keyboard cursor.
		UISubsystem->SetCursorActive(false);

		// Leaving an item only clears the hover this grid set; another grid may already hover a new one.
		if (Item || (Presenter && Presenter->GetSlotOfItem(UISubsystem->GetHoveredItem()).IsValid()))
		{
			UISubsystem->SetHoveredItem(Item);
		}
	}
}

void UInventoryGridWidget::HandleItemClicked(UInventoryItem* Item)
{
	if (UInventoryUISubsystem* UISubsystem = GetUISubsystem())
	{
		UISubsystem->SelectItem(Item);
	}
}

void UInventoryGridWidget::HandleItemViewRemoved(UInventoryItem* Item)
{
	if (UInventoryUISubsystem* UISubsystem = GetUISubsystem())
	{
		UISubsystem->ForgetItem(Item);
	}
}
