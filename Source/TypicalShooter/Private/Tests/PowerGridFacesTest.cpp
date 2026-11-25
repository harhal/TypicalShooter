// Tests for hex faces and the power grid item fragment: face directions, opposite faces, rotation (R1-R3).

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "HhGridFunctions.h"
#include "InventoryItemManifest.h"
#include "PowerGrid/HexFaces.h"
#include "PowerGrid/PowerGridFragment.h"

namespace PowerGridFacesTest
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** Screen angle of a face's neighbour (screen Y points down, so growing angle = clockwise). */
	float ScreenAngle(const EHexFace Face)
	{
		const FVector2D Point = UHhGridFunctions::HexAxialCoordToPoint(HexFaces::GetNeighborOffset(Face), 1.f);
		return FMath::RadiansToDegrees(FMath::Atan2(Point.Y, Point.X));
	}

	int32 MaskOf(std::initializer_list<EHexFace> Faces)
	{
		int32 Mask = 0;
		for (const EHexFace Face : Faces)
		{
			Mask |= HexFaces::Bit(Face);
		}
		return Mask;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerGridFacesDirectionsTest, "TypicalShooter.PowerGrid.Faces.DirectionsClockwise", PowerGridFacesTest::Flags)
bool FPowerGridFacesDirectionsTest::RunTest(const FString& Parameters)
{
	using namespace PowerGridFacesTest;
	TestEqual(TEXT("Right face points to (1,0)"), HexFaces::GetNeighborOffset(EHexFace::Right), FHhAxialCoord(1, 0));
	TestEqual(TEXT("Top-left face points to (0,-1)"), HexFaces::GetNeighborOffset(EHexFace::TopLeft), FHhAxialCoord(0, -1));
	TestEqual(TEXT("Bottom-left face points to (-1,1)"), HexFaces::GetNeighborOffset(EHexFace::BottomLeft), FHhAxialCoord(-1, 1));

	for (int32 Index = 0; Index < HexFaces::Num; ++Index)
	{
		const EHexFace Face = HexFaces::FromIndex(Index);
		const float Expected = FMath::UnwindDegrees(60.f * Index);
		TestTrue(FString::Printf(TEXT("Face %d lies 60 deg clockwise from the previous one"), Index),
			FMath::IsNearlyEqual(FMath::UnwindDegrees(ScreenAngle(Face) - Expected), 0.f, 0.1f));
		TestEqual(FString::Printf(TEXT("Face %d neighbour is adjacent"), Index),
			UHhGridFunctions::GetHexDistance(FHhAxialCoord(0, 0), HexFaces::GetNeighborOffset(Face)), 1);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerGridFacesOppositeTest, "TypicalShooter.PowerGrid.Faces.Opposite", PowerGridFacesTest::Flags)
bool FPowerGridFacesOppositeTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Right <-> Left"), HexFaces::Opposite(EHexFace::Right), EHexFace::Left);
	TestEqual(TEXT("TopLeft <-> BottomRight"), HexFaces::Opposite(EHexFace::TopLeft), EHexFace::BottomRight);
	for (int32 Index = 0; Index < HexFaces::Num; ++Index)
	{
		const EHexFace Face = HexFaces::FromIndex(Index);
		const FHhAxialCoord A = HexFaces::GetNeighborOffset(Face);
		const FHhAxialCoord B = HexFaces::GetNeighborOffset(HexFaces::Opposite(Face));
		TestEqual(FString::Printf(TEXT("Face %d and its opposite point in opposite directions"), Index),
			A.CellCoord + B.CellCoord, FIntVector2(0, 0));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerGridFacesFromOffsetTest, "TypicalShooter.PowerGrid.Faces.FaceTowardsNeighbor", PowerGridFacesTest::Flags)
bool FPowerGridFacesFromOffsetTest::RunTest(const FString& Parameters)
{
	EHexFace Face = EHexFace::Right;
	TestTrue(TEXT("(2,0) -> (1,-1) is a neighbour"), HexFaces::FaceTowards(FHhAxialCoord(1, -1), FHhAxialCoord(2, -1), Face));
	TestEqual(TEXT("... through the right face"), Face, EHexFace::Right);
	TestTrue(TEXT("(0,0) -> (0,1)"), HexFaces::FaceTowards(FHhAxialCoord(0, 0), FHhAxialCoord(0, 1), Face));
	TestEqual(TEXT("... through the bottom-right face"), Face, EHexFace::BottomRight);
	TestFalse(TEXT("Non-adjacent cells share no face"), HexFaces::FaceTowards(FHhAxialCoord(0, 0), FHhAxialCoord(2, 0), Face));
	TestFalse(TEXT("A cell is not its own neighbour"), HexFaces::FaceTowards(FHhAxialCoord(0, 0), FHhAxialCoord(0, 0), Face));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerGridFacesRotateTest, "TypicalShooter.PowerGrid.Faces.RotateClockwise", PowerGridFacesTest::Flags)
bool FPowerGridFacesRotateTest::RunTest(const FString& Parameters)
{
	using namespace PowerGridFacesTest;
	const int32 Mask = MaskOf({ EHexFace::Right, EHexFace::Left });
	TestEqual(TEXT("Zero steps keep the mask"), HexFaces::RotateMask(Mask, 0), Mask);
	TestEqual(TEXT("One clockwise step: Right -> BottomRight, Left -> TopLeft"), HexFaces::RotateMask(Mask, 1),
		MaskOf({ EHexFace::BottomRight, EHexFace::TopLeft }));
	TestEqual(TEXT("TopRight wraps around to Right"), HexFaces::RotateMask(MaskOf({ EHexFace::TopRight }), 1), MaskOf({ EHexFace::Right }));
	TestEqual(TEXT("Six steps are a full turn"), HexFaces::RotateMask(Mask, 6), Mask);
	TestEqual(TEXT("Steps wrap modulo 6"), HexFaces::RotateMask(Mask, 7), HexFaces::RotateMask(Mask, 1));
	TestEqual(TEXT("Negative steps rotate counter-clockwise"), HexFaces::RotateMask(Mask, -1), HexFaces::RotateMask(Mask, 5));
	TestEqual(TEXT("Bits outside the six faces are dropped"), HexFaces::RotateMask(0xFF, 0), HexFaces::AllFaces);
	TestEqual(TEXT("All faces stay all faces"), HexFaces::RotateMask(HexFaces::AllFaces, 3), HexFaces::AllFaces);
	TestTrue(TEXT("IsOpen reads the mask"), HexFaces::IsOpen(Mask, EHexFace::Left));
	TestFalse(TEXT("IsOpen on a closed face"), HexFaces::IsOpen(Mask, EHexFace::TopRight));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerGridFragmentDefaultsTest, "TypicalShooter.PowerGrid.Faces.FragmentDefaults", PowerGridFacesTest::Flags)
bool FPowerGridFragmentDefaultsTest::RunTest(const FString& Parameters)
{
	const FPowerGrid_ItemFragment Fragment;
	TestEqual(TEXT("Default type is connector"), Fragment.ItemType, EPowerItemType::Connector);
	TestEqual(TEXT("All faces open by default"), Fragment.OpenFaces, HexFaces::AllFaces);
	TestEqual(TEXT("Source power defaults to 1"), Fragment.Source.Power, 1);
	TestFalse(TEXT("Source needs no ammo by default"), Fragment.Source.NeedsAmmo());
	TestEqual(TEXT("Source spends 1 per use by default"), Fragment.Source.CostPerUse, 1);
	TestEqual(TEXT("Receiver needs power 1 by default"), Fragment.Receiver.RequiredPower, 1);
	TestTrue(TEXT("Receiver has no ability by default"), Fragment.Receiver.Ability == nullptr);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerGridFragmentRotationTest, "TypicalShooter.PowerGrid.Faces.FragmentRotatedFaces", PowerGridFacesTest::Flags)
bool FPowerGridFragmentRotationTest::RunTest(const FString& Parameters)
{
	using namespace PowerGridFacesTest;
	FPowerGrid_ItemFragment Fragment;
	Fragment.ItemType = EPowerItemType::Source;
	Fragment.OpenFaces = MaskOf({ EHexFace::Right });
	TestEqual(TEXT("Unrotated item keeps its faces"), Fragment.GetOpenFaces(0), MaskOf({ EHexFace::Right }));
	TestEqual(TEXT("Rotated twice: Right -> BottomLeft"), Fragment.GetOpenFaces(2), MaskOf({ EHexFace::BottomLeft }));
	TestTrue(TEXT("IsFaceOpen honours the rotation"), Fragment.IsFaceOpen(EHexFace::BottomLeft, 2));
	TestFalse(TEXT("... and closes the original face"), Fragment.IsFaceOpen(EHexFace::Right, 2));

	FInventoryItemManifest Manifest;
	Manifest.Fragments.Add(TInstancedStruct<FItemFragment>::Make<FPowerGrid_ItemFragment>(Fragment));
	const FPowerGrid_ItemFragment* Found = Manifest.GetFragmentOfType<FPowerGrid_ItemFragment>();
	if (TestNotNull(TEXT("Manifest finds the power grid fragment"), Found))
	{
		TestEqual(TEXT("Found fragment keeps its type"), Found->ItemType, EPowerItemType::Source);
	}
	return true;
}

#endif
