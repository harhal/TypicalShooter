// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryItem.h"
#include "Engine/Texture2D.h"
#include "Net/UnrealNetwork.h"

void UInventoryItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ItemManifest);
	DOREPLIFETIME(ThisClass, Rotation);
}

void UInventoryItem::SetItemManifest(const FInventoryItemManifest& NewItemManifest)
{
	ItemManifest = FInstancedStruct::Make(NewItemManifest);
}

void UInventoryItem::SetRotation(const int32 NewRotation)
{
	constexpr int32 NumSteps = 6;
	Rotation = ((NewRotation % NumSteps) + NumSteps) % NumSteps;
}

UInventoryItem* UInventoryItem::CloneFor(UObject* Outer) const
{
	UInventoryItem* Clone = NewObject<UInventoryItem>(Outer);
	Clone->ItemManifest = ItemManifest;
	Clone->Rotation = Rotation;
	return Clone;
}

const FInfo_ItemFragment* UInventoryItem::GetInfoFragment() const
{
	const FInventoryItemManifest* Manifest = GetItemManifest();
	return Manifest ? Manifest->GetFragmentOfType<FInfo_ItemFragment>() : nullptr;
}

FSlateBrush UInventoryItem::GetItemIcon() const
{
	FSlateBrush Brush;
	if (const FInfo_ItemFragment* Info = GetInfoFragment(); Info && Info->Icon)
	{
		Brush.SetResourceObject(Info->Icon);
		Brush.ImageSize = FVector2D(Info->Icon->GetSizeX(), Info->Icon->GetSizeY());
	}
	return Brush;
}

FText UInventoryItem::GetItemName() const
{
	const FInfo_ItemFragment* Info = GetInfoFragment();
	return Info ? Info->DisplayName : FText::FromString(GetName());
}

FText UInventoryItem::GetItemDescription() const
{
	const FInfo_ItemFragment* Info = GetInfoFragment();
	return Info ? Info->Description : FText::GetEmpty();
}

int32 UInventoryItem::GetItemCount() const
{
	// Stacking is not implemented yet: every item is a single unit.
	return 1;
}
