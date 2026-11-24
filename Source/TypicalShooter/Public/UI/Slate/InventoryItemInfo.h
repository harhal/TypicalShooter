// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/InventoryItem/InventoryItem_View.h"
#include "Widgets/SCompoundWidget.h"

class SImage;
class STextBlock;

/**
 * Item details panel: large icon, name and description. Empty (collapsed content) when no item is shown.
 */
class TYPICALSHOOTER_API SInventoryItemInfo : public SCompoundWidget, public IInventoryItem_View
{
public:
	SLATE_BEGIN_ARGS(SInventoryItemInfo)
		: _IconSize(128.f)
	{ }
		SLATE_ARGUMENT(float, IconSize)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual void SetIcon(const FSlateBrush& InIconBrush) override;
	virtual void SetName(const FText& InName) override;
	virtual void SetDescription(const FText& InDescription) override;
	virtual void SetCount(int32 InCount) override;

	bool HasItem() const { return Count > 0; }
	FText GetDisplayedName() const;
	FText GetDisplayedDescription() const;

private:
	FSlateBrush IconBrush;
	int32 Count = 0;

	TSharedPtr<SImage> IconImage;
	TSharedPtr<STextBlock> NameText;
	TSharedPtr<STextBlock> DescriptionText;
};
