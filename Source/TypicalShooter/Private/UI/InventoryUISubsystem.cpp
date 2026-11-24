// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/InventoryUISubsystem.h"

#include "HhGridFunctions.h"
#include "InventoryItem.h"
#include "InventoryLayout.h"
#include "UI/ItemsContainer/ItemsContainer_Model.h"

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
	if (IsCursorActive())
	{
		return GetFocusedItem();
	}
	UInventoryItem* Hovered = HoveredItem.Get();
	return Hovered ? Hovered : SelectedItem.Get();
}

void UInventoryUISubsystem::NotifyChanged()
{
	OnSelectionChanged.Broadcast();
	OnDisplayedItemChanged.Broadcast(GetDisplayedItem());
}

// --- Gamepad / keyboard cursor ----------------------------------------------------------------------------------------

namespace InventoryUISubsystemPrivate
{
	/** Minimum cosine between the wanted direction and a neighbour's direction (excludes neighbours 60 degrees off). */
	constexpr double MinDirectionScore = 0.55;
	constexpr double ScoreTolerance = 1e-3;

	bool IsInBounds(const IItemsContainer_Model* Model, const FHhAxialCoord& Location)
	{
		return Model && UHhGridFunctions::GetHexDistance(Location, FHhAxialCoord(0, 0)) <= Model->GetGridRadius();
	}

	FVector2D CellPoint(const FHhAxialCoord& Location)
	{
		return UHhGridFunctions::HexAxialCoordToPoint(Location, 1.f);
	}
}

void UInventoryUISubsystem::SetNavigableContainers(const TArray<UObject*>& Containers)
{
	UObject* Current = GetCursorContainer();
	if (Current)
	{
		RememberedCursorLocations.Add(Current, CursorLocation);
	}

	NavigableContainers.Reset();
	for (UObject* Container : Containers)
	{
		if (IsValid(Container) && Cast<IItemsContainer_Model>(Container))
		{
			NavigableContainers.AddUnique(Container);
		}
	}

	const int32 CurrentIndex = Current ? NavigableContainers.IndexOfByKey(Current) : INDEX_NONE;
	if (CurrentIndex != INDEX_NONE)
	{
		CursorContainerIndex = CurrentIndex;
	}
	else if (NavigableContainers.Num() > 0)
	{
		EnterContainer(0);
	}
	else
	{
		CursorContainerIndex = INDEX_NONE;
		bCursorActive = false;
	}
	NotifyChanged();
}

void UInventoryUISubsystem::EnterContainer(const int32 Index)
{
	CursorContainerIndex = Index;
	const FHhAxialCoord* Remembered = RememberedCursorLocations.Find(GetCursorContainer());
	CursorLocation = Remembered && InventoryUISubsystemPrivate::IsInBounds(GetCursorModel(), *Remembered) ? *Remembered : FHhAxialCoord(0, 0);
	CursorAnchorX = GetCellScreenX(CursorLocation);
}

UObject* UInventoryUISubsystem::GetCursorContainer() const
{
	return NavigableContainers.IsValidIndex(CursorContainerIndex) ? NavigableContainers[CursorContainerIndex].Get() : nullptr;
}

IItemsContainer_Model* UInventoryUISubsystem::GetCursorModel() const
{
	return Cast<IItemsContainer_Model>(GetCursorContainer());
}

double UInventoryUISubsystem::GetCellScreenX(const FHhAxialCoord& Location)
{
	return InventoryUISubsystemPrivate::CellPoint(Location).X;
}

void UInventoryUISubsystem::SetCursorActive(const bool bActive)
{
	if (bCursorActive != bActive)
	{
		bCursorActive = bActive;
		NotifyChanged();
	}
}

bool UInventoryUISubsystem::MoveCursor(const FVector2D& Direction)
{
	using namespace InventoryUISubsystemPrivate;

	const IItemsContainer_Model* Model = GetCursorModel();
	if (!Model || Direction.IsNearlyZero())
	{
		return false;
	}

	// Input y points up, grid (screen) y points down.
	const FVector2D Wanted = FVector2D(Direction.X, -Direction.Y).GetSafeNormal();

	TOptional<FHhAxialCoord> Best;
	FHhAxialCoord BestOffset;
	double BestScore = -1.0;
	double BestAnchorDistance = 0.0;
	double BestX = 0.0;
	for (const FHhAxialCoord& Offset : InventoryLayout::GetNeighborOffsets())
	{
		const FHhAxialCoord Candidate = InventoryLayout::Offset(CursorLocation, Offset);
		if (!IsInBounds(Model, Candidate))
		{
			continue;
		}

		const double Score = CellPoint(Offset).GetSafeNormal() | Wanted;
		if (Score < MinDirectionScore)
		{
			continue;
		}

		// Equally good directions (straight up / down): keep the column, then prefer the right-hand cell.
		const double X = CellPoint(Candidate).X;
		const double AnchorDistance = FMath::Abs(X - CursorAnchorX);
		const bool bBetter = !Best.IsSet()
			|| Score > BestScore + ScoreTolerance
			|| (Score > BestScore - ScoreTolerance
				&& (AnchorDistance < BestAnchorDistance - ScoreTolerance
					|| (AnchorDistance < BestAnchorDistance + ScoreTolerance && X > BestX)));
		if (bBetter)
		{
			Best = Candidate;
			BestOffset = Offset;
			BestScore = Score;
			BestAnchorDistance = AnchorDistance;
			BestX = X;
		}
	}

	if (!Best.IsSet())
	{
		SetCursorActive(true);
		return false;
	}

	CursorLocation = Best.GetValue();
	if (BestOffset.CellCoord.Y == 0)
	{
		// Horizontal moves pick a new column.
		CursorAnchorX = GetCellScreenX(CursorLocation);
	}
	bCursorActive = true;
	NotifyChanged();
	return true;
}

void UInventoryUISubsystem::SwitchCursorContainer(const int32 Step)
{
	const int32 Num = NavigableContainers.Num();
	if (Num == 0 || Step == 0)
	{
		return;
	}

	if (UObject* Current = GetCursorContainer())
	{
		RememberedCursorLocations.Add(Current, CursorLocation);
	}
	const int32 From = FMath::Max(CursorContainerIndex, 0);
	EnterContainer(((From + FMath::Sign(Step)) % Num + Num) % Num);
	bCursorActive = true;
	NotifyChanged();
}

UInventoryItem* UInventoryUISubsystem::GetItemAtCursor() const
{
	const IItemsContainer_Model* Model = GetCursorModel();
	if (!Model)
	{
		return nullptr;
	}
	for (const TPair<UInventoryItem*, FHhAxialCoord>& Placed : Model->GetPlacedItems())
	{
		if (Placed.Value == CursorLocation)
		{
			return Placed.Key;
		}
	}
	return nullptr;
}

void UInventoryUISubsystem::ConfirmAtCursor()
{
	UObject* CursorContainer = GetCursorContainer();
	if (!CursorContainer)
	{
		return;
	}
	bCursorActive = true;

	UInventoryItem* Selected = SelectedItem.Get();
	UInventoryItem* AtCursor = GetItemAtCursor();
	if (!Selected || Selected == AtCursor)
	{
		// Pick up the item under the cursor, or put the selected one back down.
		SelectedItem = Selected ? nullptr : AtCursor;
		NotifyChanged();
		return;
	}

	// Items are created with their inventory component as the outer, which is a container model.
	SelectedItem.Reset();
	if (IItemsContainer_Model* Source = Cast<IItemsContainer_Model>(Selected->GetOuter()))
	{
		Source->RequestTransferItem(Selected, TScriptInterface<IItemsContainer_Model>(CursorContainer), CursorLocation);
	}
	NotifyChanged();
}

bool UInventoryUISubsystem::CancelSelection()
{
	if (!SelectedItem.IsValid())
	{
		return false;
	}
	SelectedItem.Reset();
	NotifyChanged();
	return true;
}

UInventoryItem* UInventoryUISubsystem::GetFocusedItem() const
{
	UInventoryItem* AtCursor = IsCursorActive() ? GetItemAtCursor() : nullptr;
	return AtCursor ? AtCursor : SelectedItem.Get();
}
