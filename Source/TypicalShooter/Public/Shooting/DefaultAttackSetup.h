// Attack abilities of the starting receivers (power grid DefaultLoadout): a single-projectile ability and a
// 7-pellet one (the slot comes from the receiver slot), plus the character's starting reserve ammo effect.

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

/** Instant effect with the starting reserve ammo. Attack parameters are neutral until receivers work (power grid R5a). */
UCLASS()
class TYPICALSHOOTER_API UDefaultAttackStatsEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UDefaultAttackStatsEffect();

private:
	void AddReserve(const FGameplayTag& AmmoType, float Value);
};
