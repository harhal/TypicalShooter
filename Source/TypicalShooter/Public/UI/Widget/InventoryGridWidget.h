// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "Styling/SlateBrush.h"
#include "InventoryGridWidget.generated.h"

class APawn;
class SInventoryGrid;
class UInventoryItem;
class UInventoryUISubsystem;
class UItemsContainer_Presenter;

/**
 * UMG wrapper around SInventoryGrid. Assign a container presenter to show its items, or enable
 * bShowOwningPawnInventory to follow the inventory of the owning player's pawn.
 * Hover / click are shared with every other grid through UInventoryUISubsystem (feeds the item info panel).
 */
UCLASS()
class TYPICALSHOOTER_API UInventoryGridWidget : public UWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="InventoryGrid")
	float HexSize = 50.f;

	/** Grid radius shown in the designer / before a model is assigned. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="InventoryGrid")
	int32 PreviewGridRadius = 2;

	/** Show the UInventoryComponent of the owning player's pawn (re-binds when the pawn changes). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="InventoryGrid")
	bool bShowOwningPawnInventory = false;

	/** Background cell image (e.g. a hex material). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="InventoryGrid|Style")
	FSlateBrush CellBrush;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="InventoryGrid|Style")
	FLinearColor CellColor = FLinearColor(1.f, 1.f, 1.f, 0.25f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="InventoryGrid|Style")
	FLinearColor HoveredCellColor = FLinearColor(1.f, 1.f, 1.f, 0.3f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="InventoryGrid|Style")
	FLinearColor SelectedCellColor = FLinearColor(1.f, 0.75f, 0.2f, 0.5f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="InventoryGrid|Style")
	FLinearColor DropTargetCellColor = FLinearColor(0.3f, 1.f, 0.4f, 0.5f);

	UFUNCTION(BlueprintCallable, Category="InventoryGrid")
	void AssignPresenter(UItemsContainer_Presenter* InPresenter);

	UFUNCTION(BlueprintPure, Category="InventoryGrid")
	UItemsContainer_Presenter* GetPresenter() const { return Presenter; }

	/** Number of item views currently shown in the grid (0 if the Slate widget isn't built). */
	UFUNCTION(BlueprintPure, Category="InventoryGrid")
	int32 GetNumDisplayedItems() const;

	TSharedPtr<SInventoryGrid> GetSlateGrid() const { return SlateGrid; }

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

protected:
	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface

private:
	UFUNCTION()
	void HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

	void BindOwningPawnInventory();
	void ShowPawnInventory(APawn* Pawn);

	UInventoryUISubsystem* GetUISubsystem() const;
	void BindPresenterEvents();
	void UnbindPresenterEvents();
	void SyncSelection();

	void HandleItemHovered(UInventoryItem* Item);
	void HandleItemClicked(UInventoryItem* Item);
	void HandleItemViewRemoved(UInventoryItem* Item);

	UPROPERTY(Transient)
	TObjectPtr<UItemsContainer_Presenter> Presenter;

	TSharedPtr<SInventoryGrid> SlateGrid;

	FDelegateHandle SelectionChangedHandle;
};
