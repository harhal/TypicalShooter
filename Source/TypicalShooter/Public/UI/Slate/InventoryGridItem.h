// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/InventoryItem/InventoryItem_View.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
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
	
	void HandleHovered(bool bHovered);
	void HandleFocus(bool bFocus);
	FReply HandleDrag(const FGeometry& Geometry, const FPointerEvent& PointerEvent, bool bDrag);
		
	virtual void SetIcon(FSlateBrush IconBrush) override;
	virtual void SetName(FString InName) override;
	virtual void SetDescription(FString Description) override;
	virtual void SetCount(int32 Count) override;
	
	TSharedPtr<SImage> IconBrushWidget;
	TSharedPtr<STextBlock> NameTextBlock;
};
