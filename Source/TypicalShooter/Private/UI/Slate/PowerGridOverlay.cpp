#include "UI/Slate/PowerGridOverlay.h"

#include "HhGridFunctions.h"

#define LOCTEXT_NAMESPACE "PowerGridOverlay"

namespace PowerGridOverlay
{
	FVector2D GetCellCenter(const FHhAxialCoord& Coord, const float HexSize, const FVector2D& LocalSize)
	{
		return LocalSize * 0.5f + UHhGridFunctions::HexAxialCoordToPoint(Coord, HexSize);
	}

	TPair<FVector2D, FVector2D> GetFaceSpoke(const FVector2D& Center, const float HexSize, const EHexFace Face)
	{
		// Pointy-top hex: face N looks at 60*N degrees clockwise from the right (screen Y points down).
		const float Angle = FMath::DegreesToRadians(60.f * static_cast<int32>(Face));
		const float Apothem = HexSize * FMath::Sqrt(3.f) * 0.5f;
		return { Center, Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * Apothem };
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
