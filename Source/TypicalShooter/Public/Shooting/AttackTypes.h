// Attack slots, attack parameters and the resolved per-shot stats read from attributes.

#pragma once

#include "CoreMinimal.h"
#include "Shooting/ShotDamage.h"
#include "AttackTypes.generated.h"

/** Which attack: primary (LMB / RT) or alternate (RMB / LT). */
UENUM(BlueprintType)
enum class EAttackSlot : uint8
{
	Primary,
	Alternate,
};

/** Numeric parameter of an attack. Every slot has one attribute per value (UAttackAttributeSet). */
UENUM(BlueprintType)
enum class EAttackAttribute : uint8
{
	FireStartDistance,
	FireEndDistance,
	Ammo,
	ClipSize,
	PatternScale,
	ShotsPerBurst,
	ShotInterval,
	BurstInterval,
	ReloadTime,
	ProjectileSpeed,
	GravityScale,
	DamageLoss,
	DamagePhysical,
	DamageAcid,
	DamageElectric,
	DamagePlasma,
	DamageIce,
	DamageRadiation,
	Count UMETA(Hidden),
};

/** All parameters of one attack slot, resolved and clamped (R3) — what the attack ability works with. */
USTRUCT(BlueprintType)
struct TYPICALSHOOTER_API FAttackStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Attack") float FireStartDistance = 0.f;
	/** Never less than FireStartDistance. */
	UPROPERTY(BlueprintReadOnly, Category = "Attack") float FireEndDistance = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "Attack") int32 Ammo = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Attack") int32 ClipSize = 1;
	UPROPERTY(BlueprintReadOnly, Category = "Attack") float PatternScale = 1.f;
	UPROPERTY(BlueprintReadOnly, Category = "Attack") int32 ShotsPerBurst = 1;
	UPROPERTY(BlueprintReadOnly, Category = "Attack") float ShotInterval = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "Attack") float BurstInterval = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "Attack") float ReloadTime = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "Attack") float ProjectileSpeed = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "Attack") float GravityScale = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "Attack") float DamageLoss = 1.f;
	/** Damage per hit by type (Damage.Type.* tags). */
	UPROPERTY(BlueprintReadOnly, Category = "Attack") FShotDamage Damage;
};
