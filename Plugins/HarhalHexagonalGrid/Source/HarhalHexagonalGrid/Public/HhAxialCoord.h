// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HhAxialCoord.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType, minimalapi)
struct HARHALHEXAGONALGRID_API FHhAxialCoord
{
	GENERATED_BODY()

	FHhAxialCoord() = default;

	FHhAxialCoord(const int q, const int r);

	FHhAxialCoord(const float q, const float r);
	
	friend uint32 GetTypeHash(const FHhAxialCoord GridCell)
	{
		return GetTypeHash(GridCell.CellCoord);
	}
	
	friend bool operator==(const FHhAxialCoord& Lhs, const FHhAxialCoord& Rhs)
	{
		return Lhs.CellCoord == Rhs.CellCoord;
	}
	
	float GetQ() const;
	float GetR() const;
	float GetS() const;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntVector2 CellCoord = FIntVector2();
};