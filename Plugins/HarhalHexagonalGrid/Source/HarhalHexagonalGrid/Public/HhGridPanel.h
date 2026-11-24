// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HhAxialCoord.h"
 
class UMaterialInterface;
class UMaterialInstanceDynamic;

class HARHALHEXAGONALGRID_API SHhGridPanel : public SPanel
{
public:
	class FSlot : public TSlotBase<FSlot>
	{
	public:
		SLATE_SLOT_BEGIN_ARGS(FSlot, TSlotBase<FSlot>)
		SLATE_SLOT_END_ARGS()
	
		FSlot(const FHhAxialCoord& InCoord)
			: TSlotBase<FSlot>()
			, Coord( InCoord )
		{
		}

		void Construct(const FChildren& SlotOwner, FSlotArguments&& InArgs)
		{
			TSlotBase<FSlot>::Construct(SlotOwner, MoveTemp(InArgs));
		}

		void SetCoord(const FHhAxialCoord& InCoord)
		{
			if (InCoord != Coord)
			{
				Coord = InCoord;
				Invalidate(EInvalidateWidgetReason::Layout);
			}
		}

		FHhAxialCoord GetCoord() const
		{
			return Coord;
		}

	private:
		FHhAxialCoord Coord;
	};

	SHhGridPanel() : Children(this)
	{
	}

	virtual ~SHhGridPanel() = default;
	
	SLATE_BEGIN_ARGS(SHhGridPanel) {}
        SLATE_SLOT_ARGUMENT(FSlot, Slots)
		SLATE_ARGUMENT(float, HexSize)
		SLATE_ARGUMENT(int32, VerticalSize)
		SLATE_ARGUMENT(int32, HorizontalSize)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
 
	TPanelChildren<FSlot>::FScopedWidgetSlotArguments AddSlot(const FHhAxialCoord& Coord);
 
	void RemoveSlot(const FHhAxialCoord& Coord);

	/** Removes the slot holding the given widget. Returns true if a slot was removed. */
	bool RemoveSlot(const TSharedRef<SWidget>& SlotWidget);

	/** Finds the slot holding the given widget, nullptr if none. */
	FSlot* FindSlot(const TSharedRef<SWidget>& SlotWidget);

    void ClearChildren();
 
	void SetHexSize(float NewSize);
 
	void SetGridSize(int32 InVerticalSize, int32 InHorizontalSize);
 
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	virtual FVector2D ComputeDesiredSize(float) const override;
	virtual FChildren* GetChildren() override;
 
private:
	TPanelChildren<FSlot> Children;
 
	float HexSize = 50.f;
	int32 VerticalSize = 3;
	int32 HorizontalSize = 3;
};