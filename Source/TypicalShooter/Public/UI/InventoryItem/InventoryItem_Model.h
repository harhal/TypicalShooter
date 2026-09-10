// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InventoryItem_Model.generated.h"

UINTERFACE(MinimalAPI)
class UInventoryItem_Model : public UInterface
{
	GENERATED_BODY()
};

class IInventoryItem_Model
{
	GENERATED_BODY()
public:
	virtual FSlateBrush GetIcon() { return FSlateBrush(); }
	virtual FString GetName() { return FString(); }
	virtual FString GetDescription() {return FString(); }
	virtual int32 GetCount() { return 0; }
};
