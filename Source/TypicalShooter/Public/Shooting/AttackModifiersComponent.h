// Applies the attack modifiers (FAttackModifier_ItemFragment) of the items in the owner's inventory:
// their gameplay effects on the owner's ASC and their shot pattern replacements. Server only.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "Components/ActorComponent.h"
#include "Shooting/AttackTypes.h"
#include "Shooting/ShotPattern.h"
#include "UObject/ObjectKey.h"
#include "AttackModifiersComponent.generated.h"

class UAbilitySystemComponent;
class UInventoryComponent;
class UInventoryItem;
struct FHhAxialCoord;

UCLASS(ClassGroup = (Shooting), meta = (BlueprintSpawnableComponent))
class TYPICALSHOOTER_API UAttackModifiersComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAttackModifiersComponent();

	/** Pattern replacing Slot's own pattern (from the last held item that replaces it), or nullptr. */
	const FShotPattern* FindPatternOverride(EAttackSlot Slot) const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void HandleItemAdded(const UInventoryItem* Item, const FHhAxialCoord& Location);
	void HandleItemRemoved(const UInventoryItem* Item);
	UInventoryComponent* GetInventory() const;
	UAbilitySystemComponent* GetAbilitySystem() const;

	struct FPatternOverride
	{
		TObjectKey<UInventoryItem> Item;
		EAttackSlot Slot = EAttackSlot::Primary;
		FShotPattern Pattern;
	};

	TMap<TObjectKey<UInventoryItem>, FActiveGameplayEffectHandle> AppliedEffects;

	/** In order of arrival: the last one of a slot is active. */
	TArray<FPatternOverride> PatternOverrides;

	FDelegateHandle AddedHandle;
	FDelegateHandle RemovedHandle;
};
