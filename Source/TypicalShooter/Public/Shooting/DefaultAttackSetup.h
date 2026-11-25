// Default attacks of the player character until items define them: a rifle-like primary attack,
// a shotgun-like alternate attack, and the effect with their base attribute values (R4).

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "Shooting/AttackGameplayAbility.h"
#include "DefaultAttackSetup.generated.h"

/** Primary slot, single centred projectile. */
UCLASS()
class TYPICALSHOOTER_API UPrimaryAttackAbility : public UAttackGameplayAbility
{
	GENERATED_BODY()

public:
	UPrimaryAttackAbility();
};

/** Alternate slot, centre + ring of six projectiles. */
UCLASS()
class TYPICALSHOOTER_API UAlternateAttackAbility : public UAttackGameplayAbility
{
	GENERATED_BODY()

public:
	UAlternateAttackAbility();
};

/** Instant effect overriding the base values of both attack slots (plus the starting reserve ammo). */
UCLASS()
class TYPICALSHOOTER_API UDefaultAttackStatsEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UDefaultAttackStatsEffect();

private:
	void AddOverride(EAttackSlot Slot, EAttackAttribute Attribute, float Value);
	void AddReserve(const FGameplayTag& AmmoType, float Value);
};
