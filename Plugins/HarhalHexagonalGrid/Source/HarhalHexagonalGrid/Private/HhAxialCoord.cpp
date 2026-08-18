// Fill out your copyright notice in the Description page of Project Settings.


#include "HhAxialCoord.h"

#include "MathUtil.h"

FHhAxialCoord::FHhAxialCoord(const int q, const int r)
{
	CellCoord = FIntVector2(q, r);
}

FHhAxialCoord::FHhAxialCoord(const float q, const float r)
{
	const float s = -q-r;
	
	const int32 Round_Q = FMathf::Round(q);
	const int32 Round_R = FMathf::Round(r);
	const int32 Round_S = FMathf::Round(s);	
	
	const float Q_Diff = FMathf::Abs(Round_Q - q);
	const float R_Diff = FMathf::Abs(Round_R - r);
	const float S_Diff = FMathf::Abs(Round_S - s);

	if (Q_Diff > R_Diff and Q_Diff > S_Diff)
	{
		CellCoord = FIntVector2(-Round_R - Round_S, Round_R);
	}
	else if (R_Diff > S_Diff)
	{
		CellCoord = FIntVector2(Round_Q, -Round_Q - Round_S);
	}
	else
	{
		CellCoord = FIntVector2(Round_Q, Round_R);
	}
}

float FHhAxialCoord::GetQ() const
{
	return CellCoord.X;
}

float FHhAxialCoord::GetR() const
{
	return CellCoord.Y;
}

float FHhAxialCoord::GetS() const
{
	return -GetQ() - GetR();
}