// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryItem.h"
#include "Engine/Texture2D.h"
#include "InventoryComponent.h"
#include "Loot/LootOriginFragment.h"
#include "Loot/LootRarity.h"
#include "Net/UnrealNetwork.h"

void UInventoryItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ItemManifest);
	DOREPLIFETIME(ThisClass, Rotation);
	DOREPLIFETIME(ThisClass, Clip);
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

void UInventoryItem::OnRep_Rotation()
{
	if (UInventoryComponent* Inventory = GetTypedOuter<UInventoryComponent>())
	{
		Inventory->FlushLayoutChange();
	}
}

void UInventoryItem::SetClip(const int32 NewClip)
{
	Clip = FMath::Max(0, NewClip);
	NotifyStateChanged();
}

void UInventoryItem::OnRep_Clip()
{
	NotifyStateChanged();
}

void UInventoryItem::NotifyStateChanged()
{
	if (UInventoryComponent* Inventory = GetTypedOuter<UInventoryComponent>())
	{
		Inventory->NotifyItemStateChanged(this);
	}
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

TOptional<FItemRarityDisplay> UInventoryItem::GetItemRarity() const
{
	const FInventoryItemManifest* Manifest = GetItemManifest();
	const FLootOrigin_ItemFragment* Origin = Manifest ? Manifest->GetFragmentOfType<FLootOrigin_ItemFragment>() : nullptr;
	if (!Origin || !Origin->Rarity)
	{
		return {};
	}
	return FItemRarityDisplay{Origin->Rarity->DisplayName, Origin->Rarity->Color};
}
