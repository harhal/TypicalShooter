// Where a shot starts and what it aims at, for an avatar (character or any actor).

#pragma once

#include "CoreMinimal.h"

class AActor;

struct TYPICALSHOOTER_API FShotAim
{
	/** Projectiles start here (the avatar's "Muzzle"-tagged component, else its eyes). */
	FVector Muzzle = FVector::ZeroVector;

	/** First blocking point on the view line within the range, else the end of the range. */
	FVector AimPoint = FVector::ZeroVector;

	/** View direction (fallback when muzzle and aim point coincide). */
	FVector ViewDirection = FVector::ForwardVector;

	/** Component tag that marks the muzzle point. */
	static const FName MuzzleTag;

	/** Traces the avatar's view (Visibility channel, ignoring the avatar) up to Range. */
	static FShotAim Compute(const AActor& Avatar, float Range);
};
