// Item fragment: the item's role in the inventory power grid - type, open faces, source / receiver parameters (R1-R3).

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ItemFragment.h"
#include "PowerGrid/HexFaces.h"
#include "Abilities/GameplayAbility.h"
#include "Shooting/ShotModifier.h"
#include "PowerGridFragment.generated.h"

/** Role of an item in the power grid (R1). */
UENUM(BlueprintType)
enum class EPowerItemType : uint8
{
	/** Gives its power to every open face; accepts nothing (R7, R8). */
	Source,
	/** Grants its ability when powered in a receiver slot; passes nothing on (R2b, R5, R6). */
	Receiver,
	/** Passes power through any open face in both directions (R2b, R10). */
	Connector,
};

/** Parameters used when the item is a source (R7, R9). */
USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FPowerSourceParams
{
	GENERATED_BODY()

	/** Power given to each open face while the source works. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Source", meta = (ClampMin = 0))
	int32 Power = 1;

	/** Reserve ammo kind the source needs to work (Ammo.*); empty = always works. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Source", meta = (Categories = "Ammo"))
	FGameplayTag AmmoType;

	/** Clip size reserved from the holder's ammo. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Source", meta = (ClampMin = 0))
	int32 Capacity = 0;

	/** Ammo spent from the clip per use of a connected receiver (R9b). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Source", meta = (ClampMin = 0))
	int32 CostPerUse = 1;

	/** Idle time without uses after which the clip refills from the reserve, seconds (R9c). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Source", meta = (ClampMin = 0, Units = "s"))
	float RechargeDelay = 1.f;

	bool NeedsAmmo() const { return AmmoType.IsValid(); }
};

/** Parameters used when the item is a receiver (R5, R6). */
USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FPowerReceiverParams
{
	GENERATED_BODY()

	/** Ability granted while the receiver is powered in a slot. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Receiver")
	TSubclassOf<UGameplayAbility> Ability;

	/** Minimal incoming power for the ability to work. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Receiver", meta = (ClampMin = 0))
	int32 RequiredPower = 1;

	/** Base parameters of the ability ("the barrel"): while the receiver works in a slot they are the base values of
	 *  that slot's attack attributes; unset parameters keep the neutral defaults (R5a). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Receiver")
	TArray<FAttackStatBonus> BaseStats;
};

USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FPowerGrid_ItemFragment : public FItemFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Fragment")
	EPowerItemType ItemType = EPowerItemType::Connector;

	/** Open faces of the unrotated item (bit per EHexFace). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Fragment", meta = (Bitmask, BitmaskEnum = "/Script/TypicalShooter.EHexFace"))
	int32 OpenFaces = HexFaces::AllFaces;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Fragment", meta = (EditCondition = "ItemType == EPowerItemType::Source", EditConditionHides))
	FPowerSourceParams Source;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Fragment", meta = (EditCondition = "ItemType == EPowerItemType::Receiver", EditConditionHides))
	FPowerReceiverParams Receiver;

	/** Open faces after Rotation clockwise 60-degree steps (R3). */
	int32 GetOpenFaces(const int32 Rotation) const { return HexFaces::RotateMask(OpenFaces, Rotation); }
	bool IsFaceOpen(const EHexFace Face, const int32 Rotation) const { return HexFaces::IsOpen(GetOpenFaces(Rotation), Face); }
};
