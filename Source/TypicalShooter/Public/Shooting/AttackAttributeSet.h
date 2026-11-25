// Parameters of the attack slots as GAS attributes. Items change them with gameplay effects.
// One set holds all slots (Primary_* / Alternate_* / Air_*): GAS can't tell two instances of one set class apart.
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

	/** Clamps a parameter value. */
	static float ClampValue(EAttackAttribute Attribute, float Value);

	/** Neutral value of a parameter: what a slot has without a working receiver (power grid R5a). */
	static float GetDefaultValue(EAttackAttribute Attribute);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	// --- Primary ---
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Primary") FGameplayAttributeData Primary_FireStartDistance;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Primary") FGameplayAttributeData Primary_FireEndDistance;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Primary") FGameplayAttributeData Primary_PatternScale;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Primary") FGameplayAttributeData Primary_ShotsPerBurst;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Primary") FGameplayAttributeData Primary_ShotInterval;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Primary") FGameplayAttributeData Primary_BurstInterval;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Primary") FGameplayAttributeData Primary_ProjectileSpeed;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Primary") FGameplayAttributeData Primary_GravityScale;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Primary") FGameplayAttributeData Primary_DamageLoss;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Primary") FGameplayAttributeData Primary_Recoil;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Primary") FGameplayAttributeData Primary_DamagePhysical;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Primary") FGameplayAttributeData Primary_DamageAcid;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Primary") FGameplayAttributeData Primary_DamageElectric;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Primary") FGameplayAttributeData Primary_DamagePlasma;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Primary") FGameplayAttributeData Primary_DamageIce;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Primary") FGameplayAttributeData Primary_DamageRadiation;

	// --- Alternate ---
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alternate") FGameplayAttributeData Alternate_FireStartDistance;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alternate") FGameplayAttributeData Alternate_FireEndDistance;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alternate") FGameplayAttributeData Alternate_PatternScale;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alternate") FGameplayAttributeData Alternate_ShotsPerBurst;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alternate") FGameplayAttributeData Alternate_ShotInterval;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alternate") FGameplayAttributeData Alternate_BurstInterval;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alternate") FGameplayAttributeData Alternate_ProjectileSpeed;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alternate") FGameplayAttributeData Alternate_GravityScale;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alternate") FGameplayAttributeData Alternate_DamageLoss;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alternate") FGameplayAttributeData Alternate_Recoil;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alternate") FGameplayAttributeData Alternate_DamagePhysical;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alternate") FGameplayAttributeData Alternate_DamageAcid;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alternate") FGameplayAttributeData Alternate_DamageElectric;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alternate") FGameplayAttributeData Alternate_DamagePlasma;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alternate") FGameplayAttributeData Alternate_DamageIce;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Alternate") FGameplayAttributeData Alternate_DamageRadiation;

	// --- Air (jump while airborne, fires down) ---
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Air") FGameplayAttributeData Air_FireStartDistance;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Air") FGameplayAttributeData Air_FireEndDistance;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Air") FGameplayAttributeData Air_PatternScale;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Air") FGameplayAttributeData Air_ShotsPerBurst;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Air") FGameplayAttributeData Air_ShotInterval;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Air") FGameplayAttributeData Air_BurstInterval;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Air") FGameplayAttributeData Air_ProjectileSpeed;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Air") FGameplayAttributeData Air_GravityScale;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Air") FGameplayAttributeData Air_DamageLoss;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Air") FGameplayAttributeData Air_Recoil;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Air") FGameplayAttributeData Air_DamagePhysical;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Air") FGameplayAttributeData Air_DamageAcid;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Air") FGameplayAttributeData Air_DamageElectric;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Air") FGameplayAttributeData Air_DamagePlasma;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Air") FGameplayAttributeData Air_DamageIce;
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Air") FGameplayAttributeData Air_DamageRadiation;

private:
	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;
};
