// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"
#include "UI/InventoryItem/ItemRarityDisplay.h"

struct FItemPowerInfo;

class TYPICALSHOOTER_API IInventoryItem_View
{
public:
	virtual ~IInventoryItem_View() = default;

	virtual void SetIcon(const FSlateBrush& IconBrush) = 0;
	virtual void SetName(const FText& Name) = 0;
	virtual void SetDescription(const FText& Description) = 0;
	virtual void SetCount(int32 Count) = 0;
	/** Power grid role, power and effect steps of the item (power grid R16; empty = none). */
	virtual void SetPowerInfo(const FItemPowerInfo& Info) = 0;
	/** Unset = the item has no rarity (LootGeneration R10). */
	virtual void SetRarity(const TOptional<FItemRarityDisplay>& Rarity) = 0;
};
