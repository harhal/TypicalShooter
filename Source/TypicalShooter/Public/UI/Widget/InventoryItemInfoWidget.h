// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "InventoryItemInfoWidget.generated.h"

class SInventoryItemInfo;
class UInventoryItem;
class UInventoryItem_Presenter;

/**
 * UMG wrapper around SInventoryItemInfo. By default follows UInventoryUISubsystem: shows the hovered item,
 * otherwise the selected one, otherwise nothing.
 */
UCLASS()
class TYPICALSHOOTER_API UInventoryItemInfoWidget : public UWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ItemInfo")
	float IconSize = 128.f;

	/** Show what the owning player hovers / selects in inventory grids. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ItemInfo")
	bool bFollowInventorySelection = true;

	/** Shows Item explicitly (nullptr clears). */
	UFUNCTION(BlueprintCallable, Category="ItemInfo")
	void SetItem(UInventoryItem* Item);

	UFUNCTION(BlueprintPure, Category="ItemInfo")
	UInventoryItem* GetItem() const { return ShownItem.Get(); }

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	void HandleSelectionChanged();

	UPROPERTY(Transient)
	TObjectPtr<UInventoryItem_Presenter> Presenter;

	TWeakObjectPtr<UInventoryItem> ShownItem;
	TSharedPtr<SInventoryItemInfo> SlateInfo;
	FDelegateHandle SelectionChangedHandle;
};
