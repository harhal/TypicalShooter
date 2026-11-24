// Tests for hex grid math: axial coordinates and spiral indexing.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "HhAxialCoord.h"
#include "HhGridFunctions.h"

namespace HhGridFunctionsTest
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** Hex distance from the origin in cube coordinates. */
	int32 DistanceFromOrigin(const FHhAxialCoord& Coord)
	{
		return FMath::RoundToInt((FMath::Abs(Coord.GetQ()) + FMath::Abs(Coord.GetR()) + FMath::Abs(Coord.GetS())) / 2.f);
	}

	int32 HexDistance(const FHhAxialCoord& A, const FHhAxialCoord& B)
	{
		return HhGridFunctionsTest::DistanceFromOrigin(FHhAxialCoord(A.CellCoord.X - B.CellCoord.X, A.CellCoord.Y - B.CellCoord.Y));
	}
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHhSpiralLengthTest, "HarhalHexagonalGrid.Spiral.Length", HhGridFunctionsTest::Flags)
bool FHhSpiralLengthTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Radius 0 = centre only"), UHhGridFunctions::GetHexSpiralLength(0), 1);
	TestEqual(TEXT("Radius 1 = 7 cells"), UHhGridFunctions::GetHexSpiralLength(1), 7);
	TestEqual(TEXT("Radius 2 = 19 cells"), UHhGridFunctions::GetHexSpiralLength(2), 19);
	TestEqual(TEXT("Radius 3 = 37 cells"), UHhGridFunctions::GetHexSpiralLength(3), 37);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHhSpiralCoordsTest, "HarhalHexagonalGrid.Spiral.Coords", HhGridFunctionsTest::Flags)
bool FHhSpiralCoordsTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Index 0 is the centre"), UHhGridFunctions::MakeHexAxialCoordFromSpiral(0) == FHhAxialCoord(0, 0));

	constexpr int32 MaxRadius = 3;
	TSet<FHhAxialCoord> Seen;
	for (int32 Radius = 1; Radius <= MaxRadius; ++Radius)
	{
		const int32 First = UHhGridFunctions::GetHexSpiralLength(Radius - 1);
		const int32 Last = UHhGridFunctions::GetHexSpiralLength(Radius) - 1;
		for (int32 N = First; N <= Last; ++N)
		{
			const FHhAxialCoord Coord = UHhGridFunctions::MakeHexAxialCoordFromSpiral(N);
			TestEqual(FString::Printf(TEXT("Index %d lies on ring %d"), N, Radius), HhGridFunctionsTest::DistanceFromOrigin(Coord), Radius);

			// Each ring is walked cell by cell and closes on itself.
			const FHhAxialCoord Next = UHhGridFunctions::MakeHexAxialCoordFromSpiral(N == Last ? First : N + 1);
			TestEqual(FString::Printf(TEXT("Index %d is adjacent to the next cell of its ring"), N), HhGridFunctionsTest::HexDistance(Coord, Next), 1);

			bool bAlreadySeen = false;
			Seen.Add(Coord, &bAlreadySeen);
			TestFalse(FString::Printf(TEXT("Index %d is a new cell"), N), bAlreadySeen);
		}
	}
	TestFalse(TEXT("Centre is never repeated"), Seen.Contains(FHhAxialCoord(0, 0)));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHhAxialRoundingTest, "HarhalHexagonalGrid.AxialCoord.Rounding", HhGridFunctionsTest::Flags)
bool FHhAxialRoundingTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Near centre rounds to (0,0)"), FHhAxialCoord(0.1f, 0.1f) == FHhAxialCoord(0, 0));
	TestTrue(TEXT("Mostly +q rounds to (1,0)"), FHhAxialCoord(0.9f, 0.05f) == FHhAxialCoord(1, 0));
	TestTrue(TEXT("Mostly +r rounds to (0,1)"), FHhAxialCoord(-0.2f, 1.1f) == FHhAxialCoord(0, 1));
	TestTrue(TEXT("Negative coords round too"), FHhAxialCoord(-1.1f, 0.95f) == FHhAxialCoord(-1, 1));
	TestEqual(TEXT("S = -Q - R"), FHhAxialCoord(2, -3).GetS(), 1.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHhAxialToPointTest, "HarhalHexagonalGrid.AxialCoord.ToPoint", HhGridFunctionsTest::Flags)
bool FHhAxialToPointTest::RunTest(const FString& Parameters)
{
	constexpr float Radius = 10.f;
	TestTrue(TEXT("Origin maps to (0,0)"),
		UHhGridFunctions::HexAxialCoordToPoint(FHhAxialCoord(0, 0), Radius).Equals(FVector2D::ZeroVector, 1e-3));
	TestTrue(TEXT("+q neighbour is one hex width to the right"),
		UHhGridFunctions::HexAxialCoordToPoint(FHhAxialCoord(1, 0), Radius).Equals(FVector2D(FMath::Sqrt(3.f) * Radius, 0.f), 1e-3));
	TestTrue(TEXT("+r neighbour is half a width right and 1.5 radii down"),
		UHhGridFunctions::HexAxialCoordToPoint(FHhAxialCoord(0, 1), Radius).Equals(FVector2D(FMath::Sqrt(3.f) / 2.f * Radius, 1.5f * Radius), 1e-3));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
