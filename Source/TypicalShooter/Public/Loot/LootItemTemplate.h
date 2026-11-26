// Loot item template: describes a kind of item whose parameters are rolled by LootGenerator (LootGeneration R1).

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemFragment.h"
#include "Loot/LootPowerParams.h"
#include "PowerGrid/HexFaces.h"
#include "PowerGrid/PowerGridFragment.h"
#include "StructUtils/InstancedStruct.h"
#include "LootItemTemplate.generated.h"

/** One possible arrangement of open faces, picked by weight. */
USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FLootFaceShape
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (Bitmask, BitmaskEnum = "/Script/TypicalShooter.EHexFace"))
	int32 OpenFaces = HexFaces::AllFaces;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (ClampMin = 0))
	float Weight = 1.f;
};

UCLASS(BlueprintType)
class TYPICALSHOOTER_API ULootItemTemplate : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Role in the power grid; never random. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power Grid")
	EPowerItemType ItemType = EPowerItemType::Connector;

	/** Open face masks to pick from by weight; empty = all faces open. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power Grid")
	TArray<FLootFaceShape> FaceShapes;

	/** Rotate the picked shape by a random multiple of 60 degrees. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power Grid")
	bool bRandomRotation = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power Grid", meta = (EditCondition = "ItemType == EPowerItemType::Source", EditConditionHides))
	FLootSourceParams Source;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power Grid", meta = (EditCondition = "ItemType == EPowerItemType::Receiver", EditConditionHides))
	FLootReceiverParams Receiver;

	/** Copied into every generated item as is. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (ExcludeBaseStruct))
	TArray<TInstancedStruct<FItemFragment>> FixedFragments;
};
