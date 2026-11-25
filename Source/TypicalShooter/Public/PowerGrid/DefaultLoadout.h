// The character's starting items (power grid R6a: base attacks come from the starting set): a source in the
// centre, a rifle-like receiver in the primary slot and a shotgun-like receiver in the alternate slot.

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

	TYPICALSHOOTER_API TArray<FStartingInventoryItem> MakeStartingItems();
}
