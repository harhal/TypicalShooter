// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/InventoryItem/InventoryItem_View.h"
#include "Widgets/SCompoundWidget.h"

class SImage;
class STextBlock;

/**
 * Visual of one item in SInventoryGrid: icon with a short name. Input is handled by the grid.
 */
class TYPICALSHOOTER_API SInventoryGridItem : public SCompoundWidget, public IInventoryItem_View
{
public:
	SLATE_BEGIN_ARGS(SInventoryGridItem)
		{
		}

	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	virtual void SetIcon(const FSlateBrush& InIconBrush) override;
	virtual void SetName(const FText& InName) override;
	virtual void SetDescription(const FText& InDescription) override;
	virtual void SetCount(int32 InCount) override;

	const FSlateBrush& GetIconBrush() const { return IconBrush; }

private:
	/** SImage keeps a pointer to the brush, so the brush must live as long as the widget. */
	FSlateBrush IconBrush;
	int32 Count = 0;

	TSharedPtr<SImage> IconBrushWidget;
	TSharedPtr<STextBlock> NameTextBlock;
};
