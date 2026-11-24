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

	/** First fragment of type T whose tag matches exactly. */
	template<typename T>
	requires std::derived_from<T, FItemFragment>
	const T* GetFragment(const FGameplayTag& FragmentTag) const;

	/** First fragment of type T (or derived from it), regardless of tag. */
	template<typename T>
	requires std::derived_from<T, FItemFragment>
	const T* GetFragmentOfType() const;

	UPROPERTY(EditAnywhere, Category = Inventory, meta = (ExcludeBaseStruct))
	TArray<TInstancedStruct<FItemFragment>> Fragments;
};

template <typename T>
requires std::derived_from<T, FItemFragment>
const T* FInventoryItemManifest::GetFragment(const FGameplayTag& FragmentTag) const
{
	for (const TInstancedStruct<FItemFragment>& Fragment : Fragments)
	{
		const T* TFragment = Fragment.GetPtr<T>();
		if (TFragment && TFragment->GetFragmentTag().MatchesTagExact(FragmentTag))
		{
			return TFragment;
		}
	}

	return nullptr;
}

template <typename T>
requires std::derived_from<T, FItemFragment>
const T* FInventoryItemManifest::GetFragmentOfType() const
{
	for (const TInstancedStruct<FItemFragment>& Fragment : Fragments)
	{
		if (const T* TFragment = Fragment.GetPtr<T>())
		{
			return TFragment;
		}
	}

	return nullptr;
}
