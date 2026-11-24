// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "ItemFragment.generated.h"

class UTexture2D;

/**
 * 
 */
USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FItemFragment
{
	GENERATED_BODY()
	
	FItemFragment() {}
	
	FItemFragment(const FItemFragment&) = default;
	FItemFragment& operator=(const FItemFragment&) = default;
	FItemFragment(FItemFragment&&) = default;
	FItemFragment& operator=(FItemFragment&&) = default;
	virtual ~FItemFragment() {}
	
	const FGameplayTag& GetFragmentTag() const { return FragmentTag; }
	void SetFragmentTag(const FGameplayTag& InFragmentTag) { FragmentTag = InFragmentTag; }
	
private:
	UPROPERTY(EditAnywhere, Category = "Item Fragment")
	FGameplayTag FragmentTag = FGameplayTag::EmptyTag;
};

USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FGrid_ItemFragment : public FItemFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Item Fragment")
	int32 Size = 1;
};

/** Display data shown by inventory UI. */
USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FInfo_ItemFragment : public FItemFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Item Fragment")
	FText DisplayName;

	UPROPERTY(EditAnywhere, Category = "Item Fragment", meta = (MultiLine = true))
	FText Description;

	UPROPERTY(EditAnywhere, Category = "Item Fragment")
	TObjectPtr<UTexture2D> Icon = nullptr;
};