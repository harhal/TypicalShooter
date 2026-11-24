// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemManifest.h"
#include "StructUtils/InstancedStruct.h"
#include "UI/InventoryItem/InventoryItem_Model.h"
#include "UObject/Object.h"
#include "InventoryItem.generated.h"

/**
 * Runtime item instance. Replicated as a subobject of the owning UInventoryComponent.
 */
UCLASS()
class TYPICALSHOOTER_API UInventoryItem : public UObject, public IInventoryItem_Model
{
	GENERATED_BODY()

public:
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	void SetItemManifest(const FInventoryItemManifest& NewItemManifest);
	const FInventoryItemManifest* GetItemManifest() const { return ItemManifest.GetPtr<FInventoryItemManifest>(); }

	virtual FSlateBrush GetItemIcon() const override;
	virtual FText GetItemName() const override;
	virtual FText GetItemDescription() const override;
	virtual int32 GetItemCount() const override;

protected:
	UPROPERTY(VisibleAnywhere, meta = (BaseStruct = "/Script/TypicalShooter.InventoryItemManifest"), Replicated)
	FInstancedStruct ItemManifest;

private:
	const FInfo_ItemFragment* GetInfoFragment() const;
};
