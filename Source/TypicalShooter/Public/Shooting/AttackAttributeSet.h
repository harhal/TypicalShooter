// Parameters of the primary and alternate attacks as GAS attributes. Items change them with gameplay effects.
// One set holds both slots (Primary_* / Alternate_*): GAS can't tell two instances of one set class apart.
// Access attributes generically with GetAttribute(Slot, Attribute).

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "Shooting/AttackTypes.h"
#include "AttackAttributeSet.generated.h"

UCLASS()
class TYPICALSHOOTER_API UAttackAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UAttackAttributeSet();

	/** Attribute of a slot's parameter (invalid for EAttackAttribute::Count). */
	static FGameplayAttribute GetAttribute(EAttackSlot Slot, EAttackAttribute Attribute);

	/** Reads all parameters of Slot from Asc, clamped (R3). */
	static FAttackStats ReadStats(const UAbilitySystemComponent& Asc, EAttackSlot Slot);

	/** Adds Value to one parameter of resolved stats (damage parameters go to Stats.Damage), re-clamped. */
	static void AddToStats(FAttackStats& Stats, EAttackAttribute Attribute, float Value);

	/** Clamps a parameter value; ClipSize bounds Ammo. */
	static float ClampValue(EAttackAttribute Attribute, float Value, float ClipSize);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	// --- Primary ---
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Primary") FGameplayAttributeData Primary_FireStartDistance;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Primary") FGameplayAttributeData Primary_FireEndDistance;
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Primary_Ammo, Category = "Primary") FGameplayAttributeData Primary_Ammo;
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Primary_ClipSize, Category = "Primary") FGameplayAttributeData Primary_ClipSize;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Primary") FGameplayAttributeData Primary_PatternScale;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Primary") FGameplayAttributeData Primary_ShotsPerBurst;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Primary") FGameplayAttributeData Primary_ShotInterval;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Primary") FGameplayAttributeData Primary_BurstInterval;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Primary") FGameplayAttributeData Primary_ReloadTime;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Primary") FGameplayAttributeData Primary_ProjectileSpeed;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Primary") FGameplayAttributeData Primary_GravityScale;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Primary") FGameplayAttributeData Primary_DamageLoss;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Primary") FGameplayAttributeData Primary_DamagePhysical;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Primary") FGameplayAttributeData Primary_DamageAcid;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Primary") FGameplayAttributeData Primary_DamageElectric;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Primary") FGameplayAttributeData Primary_DamagePlasma;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Primary") FGameplayAttributeData Primary_DamageIce;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Primary") FGameplayAttributeData Primary_DamageRadiation;

	// --- Alternate ---
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alternate") FGameplayAttributeData Alternate_FireStartDistance;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alternate") FGameplayAttributeData Alternate_FireEndDistance;
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Alternate_Ammo, Category = "Alternate") FGameplayAttributeData Alternate_Ammo;
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Alternate_ClipSize, Category = "Alternate") FGameplayAttributeData Alternate_ClipSize;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alternate") FGameplayAttributeData Alternate_PatternScale;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alternate") FGameplayAttributeData Alternate_ShotsPerBurst;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alternate") FGameplayAttributeData Alternate_ShotInterval;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alternate") FGameplayAttributeData Alternate_BurstInterval;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alternate") FGameplayAttributeData Alternate_ReloadTime;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alternate") FGameplayAttributeData Alternate_ProjectileSpeed;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alternate") FGameplayAttributeData Alternate_GravityScale;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alternate") FGameplayAttributeData Alternate_DamageLoss;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alternate") FGameplayAttributeData Alternate_DamagePhysical;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alternate") FGameplayAttributeData Alternate_DamageAcid;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alternate") FGameplayAttributeData Alternate_DamageElectric;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alternate") FGameplayAttributeData Alternate_DamagePlasma;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alternate") FGameplayAttributeData Alternate_DamageIce;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alternate") FGameplayAttributeData Alternate_DamageRadiation;

protected:
	// Ammo / clip size notify on clients (HUD); other parameters replicate without change events.
	UFUNCTION() void OnRep_Primary_Ammo(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_Primary_ClipSize(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_Alternate_Ammo(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_Alternate_ClipSize(const FGameplayAttributeData& OldValue);

private:
	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;
};
