// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryItem.h"
#include "Net/UnrealNetwork.h"

void UInventoryItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, ItemManifest);
}

void UInventoryItem::SetItemManifest(const FInventoryItemManifest& NewItemManifest)
{
	ItemManifest = FInstancedStruct::Make(NewItemManifest);
}

FSlateBrush UInventoryItem::GetIcon()
{
	return IInventoryItem_Model::GetIcon();
}

FString UInventoryItem::GetName()
{
	return IInventoryItem_Model::GetName();
}

FString UInventoryItem::GetDescription()
{
	return IInventoryItem_Model::GetDescription();
}

int32 UInventoryItem::GetCount()
{
	return IInventoryItem_Model::GetCount();
}
