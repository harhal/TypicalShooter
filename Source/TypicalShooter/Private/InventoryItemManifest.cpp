// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryItemManifest.h"
#include "InventoryItem.h"

UInventoryItem* FInventoryItemManifest::ManifestItem(UObject* Outer) const
{
	UInventoryItem* Item = NewObject<UInventoryItem>(Outer);
	Item->SetItemManifest(*this);
	return Item;
}
