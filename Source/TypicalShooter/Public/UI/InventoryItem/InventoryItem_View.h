// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"

class TYPICALSHOOTER_API IInventoryItem_View
{
public:
	virtual ~IInventoryItem_View() = default;

	virtual void SetIcon(const FSlateBrush& IconBrush) = 0;
	virtual void SetName(const FText& Name) = 0;
	virtual void SetDescription(const FText& Description) = 0;
	virtual void SetCount(int32 Count) = 0;
};
