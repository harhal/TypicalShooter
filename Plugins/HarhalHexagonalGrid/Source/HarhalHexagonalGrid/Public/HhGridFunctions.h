// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HhAxialCoord.h"
#include "HhGridFunctions.generated.h"

/**
 * 
 */
UCLASS()
class HARHALHEXAGONALGRID_API UHhGridFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure)
	static FHhAxialCoord MakeHexAxialCoordFromSpiral(const int32 n);
	
	UFUNCTION(BlueprintPure)
	static int32 GetHexSpiralLength(const int32 r);
	
	UFUNCTION(BlueprintPure)
	static FVector2D HexAxialCoordToPoint(const FHhAxialCoord& Coord, const float CellBigRadius);
};
