#include "UI/Slate/PowerGridOverlay.h"

#include "HhGridFunctions.h"

#define LOCTEXT_NAMESPACE "PowerGridOverlay"

namespace PowerGridOverlay
{
	/** Fraction of the centre-to-edge distance the face stroke is drawn at (inside the cell outline). */
	constexpr float FaceInset = 0.88f;
	/** Fraction of the edge length the face stroke covers. */
	constexpr float FaceLength = 0.5f;

	FVector2D GetCellCenter(const FHhAxialCoord& Coord, const float HexSize, const FVector2D& LocalSize)
	{
		return LocalSize * 0.5f + UHhGridFunctions::HexAxialCoordToPoint(Coord, HexSize);
	}

	TPair<FVector2D, FVector2D> GetFaceSegment(const FVector2D& Center, const float HexSize, const EHexFace Face)
	{
		// Pointy-top hex: face N looks at 60*N degrees clockwise from the right (screen Y points down); its edge
		// runs between the corners at +-30 degrees around that direction.
		const float Angle = FMath::DegreesToRadians(60.f * static_cast<int32>(Face));
		const FVector2D Normal(FMath::Cos(Angle), FMath::Sin(Angle));
		const FVector2D Along(-Normal.Y, Normal.X);
		const float Apothem = HexSize * FMath::Sqrt(3.f) * 0.5f;
		const FVector2D Middle = Center + Normal * (Apothem * FaceInset);
		const FVector2D HalfStroke = Along * (HexSize * FaceLength * 0.5f);
		return { Middle - HalfStroke, Middle + HalfStroke };
	}

	FText GetPowerLabel(const FPowerCellDisplay& Cell)
	{
		FText Label = Cell.Type == EPowerItemType::Receiver
			? FText::Format(LOCTEXT("ReceiverPower", "{0}/{1}"), Cell.Power, Cell.RequiredPower)
			: FText::AsNumber(Cell.Power);
		if (Cell.Clip.IsSet())
		{
			Label = FText::Format(LOCTEXT("Clip", "{0} [{1}/{2}]"), Label, FText::AsNumber(Cell.Clip->X), FText::AsNumber(Cell.Clip->Y));
		}
		return Label;
	}

	FText GetSlotLabel(const FHhAxialCoord& Location)
	{
		const TOptional<EReceiverSlot> Slot = ReceiverSlots::GetSlotAt(Location);
		if (!Slot.IsSet())
		{
			return FText::GetEmpty();
		}
		switch (Slot.GetValue())
		{
		case EReceiverSlot::Primary:   return LOCTEXT("PrimarySlot", "LMB");
		case EReceiverSlot::Alternate: return LOCTEXT("AlternateSlot", "RMB");
		case EReceiverSlot::Air:       return LOCTEXT("AirSlot", "AIR");
		}
		return FText::GetEmpty();
	}
}

#undef LOCTEXT_NAMESPACE
