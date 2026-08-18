// Fill out your copyright notice in the Description page of Project Settings.

#include "HhGridPlane.h"

FHhAxialCoord UHhGridPlane::GetAxialCellByPoint(FVector Point) const
{
	const FVector& LocalPoint = WorldToLocalPoint(Point);
	return GetAxialCellByLocalPoint(LocalPoint);
}

FVector UHhGridPlane::GetCellCenterLocation(const FHhAxialCoord& Cell) const
{
	const FVector& LocalCenter = GetCellCenterLocalLocation(Cell);
	return LocalToWorldPoint(LocalCenter);
}

FVector UHhGridPlane::LocalToWorldPoint(const FVector& Point) const
{
	return GetComponentTransform().TransformPositionNoScale((Point - GetLocalCenterOffset()) * CellSize);
}

FVector UHhGridPlane::WorldToLocalPoint(const FVector& Point) const
{
	return GetComponentTransform().InverseTransformPositionNoScale(Point) / CellSize + GetLocalCenterOffset();
}

FHhAxialCoord UHhGridPlane::GetAxialCellByLocalPoint(const FVector& Point)
{
	const FVector ScaledPoint = Point / LocalCellLongestRadius;

	const float q = (FMath::Sqrt(3.f) * ScaledPoint.X  -  ScaledPoint.Y) / 3.f;
	const float r = 2.f * ScaledPoint.Y / 3.f;
    return FHhAxialCoord(q, r);
}

FVector UHhGridPlane::GetCellCenterLocalLocation(const FHhAxialCoord& Cell)
{    
	const float x = FMath::Sqrt(3.f) * Cell.GetQ()  +  sqrt(3)/2 * Cell.GetR();
	const float y = 3.f/2.f * Cell.GetR();

	return FVector(x, y, 0.f) * LocalCellLongestRadius;
}

float UHhGridPlane::GetCellsLocalHorizontalHalfOffset()
{
	return LocalCellLongestRadius * FMath::Cos(AngleBetweenLongestAndShortestRadius);
}

float UHhGridPlane::GetCellsLocalVerticalOffset()
{
	return LocalCellLongestRadius * (1 + FMath::Sin(AngleBetweenLongestAndShortestRadius));
}

FVector UHhGridPlane::GetLocalCenterOffset() const
{
	if (!bAutoCenter)
	{
		return FVector::ZeroVector;
	}

	const FHhAxialCoord& LastCell = FHhAxialCoord(GridSize.X - 1, GridSize.Y - 1);
	return GetCellCenterLocalLocation(LastCell) / 2;
}
