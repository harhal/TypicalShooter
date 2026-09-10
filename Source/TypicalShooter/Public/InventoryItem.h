// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemManifest.h"
#include "StructUtils/InstancedStruct.h"
#include "UI/InventoryItem/InventoryItem_Model.h"
#include "UObject/Object.h"
#include "InventoryItem.generated.h"

/**
 * 
 */
UCLASS()
class TYPICALSHOOTER_API UInventoryItem : public UObject, public IInventoryItem_Model
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	void SetItemManifest(const FInventoryItemManifest& NewItemManifest);
	
	virtual FSlateBrush GetIcon() override;
	virtual FString GetName() override;
	virtual FString GetDescription() override;
	virtual int32 GetCount() override;
	
protected:
	UPROPERTY(VisibleAnywhere, meta = (BaseStruct = "/Script/TypicalShooter.InventoryItemManifest"), Replicated)
	FInstancedStruct ItemManifest;
};
