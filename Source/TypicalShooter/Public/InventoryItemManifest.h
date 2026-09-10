// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemFragment.h"
#include "StructUtils/InstancedStruct.h"
#include "UObject/Object.h"
#include "InventoryItemManifest.generated.h"

class UInventoryItem;
/**
 * 
 */
USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FInventoryItemManifest
{
	GENERATED_BODY()
	
public:
	UInventoryItem* ManifestItem(UObject* Outer) const;
	
	template<typename T>
	requires std::derived_from<T, FItemFragment>
	T* GetFragment(const FGameplayTag& FragmentTag) const;
	
	UPROPERTY(EditAnywhere, Category = Inventory, meta = (ExcludeBaseStruct))
	TArray<TInstancedStruct<FItemFragment>> Fragments;
};

template <typename T> 
requires std::derived_from<T, FItemFragment>
T* FInventoryItemManifest::GetFragment(const FGameplayTag& FragmentTag) const
{
	for (TInstancedStruct<FItemFragment> Fragment : Fragments)
	{
		T* TFragment = Fragment.GetPtr<T>();
		if (!TFragment)
		{
			continue;
		}
		
		if (!TFragment->GetFragmentTag().MatchesTagExact(FragmentTag))
		{
			continue;
		}
		
		return &Fragment.Get();
	}
	
	return nullptr;
}
