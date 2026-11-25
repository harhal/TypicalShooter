// Clips of the power sources in the owner's inventory (power grid R9, R9d): a source needing ammo reserves up to its
// capacity from the owner's reserve (UAmmoAttributeSet) when it enters the inventory and returns the rest when it
// leaves. Server only; needs an ASC with ammo reserves on the owner.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SourceAmmoComponent.generated.h"

class UAbilitySystemComponent;
class UInventoryComponent;
class UInventoryItem;
struct FHhAxialCoord;

UCLASS(ClassGroup = (PowerGrid), meta = (BlueprintSpawnableComponent))
class TYPICALSHOOTER_API USourceAmmoComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USourceAmmoComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

private:
	void HandleItemAdded(const UInventoryItem* Item, const FHhAxialCoord& Location);
	void HandleItemRemoved(const UInventoryItem* Item);
	/** Tops Item's clip up to its capacity from the reserve (as much as there is); no-op for other items. */
	void Refill(UInventoryItem* Item) const;

	TWeakObjectPtr<UInventoryComponent> Inventory;
	TWeakObjectPtr<UAbilitySystemComponent> Asc;
	FDelegateHandle AddedHandle;
	FDelegateHandle RemovedHandle;
};
