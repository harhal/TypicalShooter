// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "InventoryGridWidget.generated.h"

class SInventoryGrid;
class UItemsContainer_Presenter;

/**
 * UMG wrapper around SInventoryGrid. Assign a container presenter to show its items.
 */
UCLASS()
class TYPICALSHOOTER_API UInventoryGridWidget : public UWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="InventoryGrid")
	float HexSize = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="InventoryGrid")
	int32 PreviewGridSize = 3;

	UFUNCTION(BlueprintCallable, Category="InventoryGrid")
	void AssignPresenter(UItemsContainer_Presenter* InPresenter);

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

protected:
	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface

private:
	UPROPERTY(Transient)
	TObjectPtr<UItemsContainer_Presenter> Presenter;

	TSharedPtr<SInventoryGrid> SlateGrid;
};
