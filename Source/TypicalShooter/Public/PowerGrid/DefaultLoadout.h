// The character's starting items (power grid R6a: base attacks come from the starting set): a source in the
// centre, a rifle-like receiver in the primary slot, a shotgun-like receiver in the alternate slot and a double jump
// receiver in the air slot.

#pragma once

#include "CoreMinimal.h"
#include "HhAxialCoord.h"
#include "InventoryItemManifest.h"
#include "DefaultLoadout.generated.h"

/** An item placed into the character's inventory on the server at BeginPlay. */
USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FStartingInventoryItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	FInventoryItemManifest Manifest;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	FHhAxialCoord Location;
};

namespace DefaultLoadout
{
	/** Source (power 1, all faces, no ammo) at the centre. */
	TYPICALSHOOTER_API FInventoryItemManifest MakeStarterSource();
	/** Receiver repeating the old primary attack: automatic rifle, 10 physical, 8000 cm. */
	TYPICALSHOOTER_API FInventoryItemManifest MakeRifleReceiver();
	/** Receiver repeating the old alternate attack: 7-pellet shotgun, 6 physical per pellet, 3000 cm. */
	TYPICALSHOOTER_API FInventoryItemManifest MakeShotgunReceiver();
	/** Recoil-only receiver (no projectile, 900 cm/s away from the aim): in the air slot it is a double jump. */
	TYPICALSHOOTER_API FInventoryItemManifest MakeDoubleJumpReceiver();

	TYPICALSHOOTER_API TArray<FStartingInventoryItem> MakeStartingItems();
}
