// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

class TYPICALSHOOTER_API IInventoryItem_View
{
public:
    virtual ~IInventoryItem_View() = default;	
		
	virtual void SetIcon(FSlateBrush IconBrush) = 0;
	virtual void SetName(FString Name) = 0;
	virtual void SetDescription(FString Description) = 0;
	virtual void SetCount(int32 Count) = 0;
};