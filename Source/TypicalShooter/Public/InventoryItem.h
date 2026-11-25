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

	/** Clockwise 60-degree steps, 0..5 (power grid R3). Server-side state, replicated. */
	int32 GetRotation() const { return Rotation; }
	void SetRotation(int32 NewRotation);

	/** Ammo held by a power source (power grid R9). Server-side state, replicated; never negative. */
	int32 GetClip() const { return Clip; }
	/** Sets the clip and reports the change through the owning inventory's item state event. */
	void SetClip(int32 NewClip);

	/**
	 * New item with the same manifest and instance state, owned by Outer (for moving between inventories, R3a).
	 * The clip is not carried: a source returns it to the reserve on leaving and reserves anew on entering (R9d).
	 */
	UInventoryItem* CloneFor(UObject* Outer) const;

protected:
	UPROPERTY(VisibleAnywhere, meta = (BaseStruct = "/Script/TypicalShooter.InventoryItemManifest"), Replicated)
	FInstancedStruct ItemManifest;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Rotation)
	int32 Rotation = 0;

	/** Clients: report the rotation through the owning inventory's layout event. */
	UFUNCTION()
	void OnRep_Rotation();

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Clip)
	int32 Clip = 0;

	/** Clients: report the clip through the owning inventory's item state event. */
	UFUNCTION()
	void OnRep_Clip();

	void NotifyStateChanged();

private:
	const FInfo_ItemFragment* GetInfoFragment() const;
};
