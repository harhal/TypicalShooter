// Default attacks of the player character until items define them: a rifle-like primary attack,
// a shotgun-like alternate attack, a reload per slot and the effect with their base attribute values (R4).

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "Shooting/AttackGameplayAbility.h"
#include "Shooting/ReloadGameplayAbility.h"
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

UCLASS()
class TYPICALSHOOTER_API UPrimaryReloadAbility : public UReloadGameplayAbility
{
	GENERATED_BODY()

public:
	UPrimaryReloadAbility();
};

UCLASS()
class TYPICALSHOOTER_API UAlternateReloadAbility : public UReloadGameplayAbility
{
	GENERATED_BODY()

public:
	UAlternateReloadAbility();
};

/** Instant effect overriding the base values of both attack slots (clip sizes before ammo). */
UCLASS()
class TYPICALSHOOTER_API UDefaultAttackStatsEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UDefaultAttackStatsEffect();

private:
	void AddOverride(EAttackSlot Slot, EAttackAttribute Attribute, float Value);
};
