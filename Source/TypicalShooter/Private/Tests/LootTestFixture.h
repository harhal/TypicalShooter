// Shared helpers for TypicalShooter.Loot.* tests.

#pragma once

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "InventoryItemManifest.h"
#include "Loot/LootItemTemplate.h"
#include "Loot/LootOriginFragment.h"
#include "PowerGrid/PowerGridFragment.h"

namespace LootTestFixture
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	inline int32 MaskOf(std::initializer_list<EHexFace> Faces)
	{
		int32 Mask = 0;
		for (const EHexFace Face : Faces)
		{
			Mask |= HexFaces::Bit(Face);
		}
		return Mask;
	}

	inline ULootItemTemplate* MakeTemplate(const EPowerItemType Type, std::initializer_list<FLootFaceShape> Shapes, const bool bRandomRotation)
	{
		ULootItemTemplate* Template = NewObject<ULootItemTemplate>();
		Template->ItemType = Type;
		Template->FaceShapes = Shapes;
		Template->bRandomRotation = bRandomRotation;
		return Template;
	}

	inline FLootFaceShape Shape(const int32 Mask, const float Weight = 1.f)
	{
		FLootFaceShape Result;
		Result.OpenFaces = Mask;
		Result.Weight = Weight;
		return Result;
	}

	inline const FPowerGrid_ItemFragment* PowerGridOf(const FInventoryItemManifest& Manifest)
	{
		return Manifest.GetFragmentOfType<FPowerGrid_ItemFragment>();
	}
	/** A pointer into a temporary manifest would dangle: keep the manifest in a local first. */
	const FPowerGrid_ItemFragment* PowerGridOf(FInventoryItemManifest&&) = delete;

	/** Open faces of the generated item; INDEX_NONE if it has no power grid fragment. */
	inline int32 OpenFacesOf(const FInventoryItemManifest& Manifest)
	{
		const FPowerGrid_ItemFragment* PowerGrid = PowerGridOf(Manifest);
		return PowerGrid ? PowerGrid->OpenFaces : INDEX_NONE;
	}

	inline const FLootOrigin_ItemFragment* OriginOf(const FInventoryItemManifest& Manifest)
	{
		return Manifest.GetFragmentOfType<FLootOrigin_ItemFragment>();
	}
	const FLootOrigin_ItemFragment* OriginOf(FInventoryItemManifest&&) = delete;
}

#endif
