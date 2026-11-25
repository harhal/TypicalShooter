// Reserve ammo of the character, one attribute per ammo kind (R15). Spent by items (ammo-gated modifiers; power grid sources later).

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "GameplayTagContainer.h"
#include "AmmoAttributeSet.generated.h"

UCLASS()
class TYPICALSHOOTER_API UAmmoAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	/** Reserve attribute of an ammo kind (Ammo.* tag); invalid for unknown kinds. */
	static FGameplayAttribute GetReserveAttribute(const FGameplayTag& AmmoType);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Reserve_Bullet, Category = "Ammo") FGameplayAttributeData Reserve_Bullet;
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Reserve_Battery, Category = "Ammo") FGameplayAttributeData Reserve_Battery;
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Reserve_Grenade, Category = "Ammo") FGameplayAttributeData Reserve_Grenade;
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Reserve_Radiator, Category = "Ammo") FGameplayAttributeData Reserve_Radiator;
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Reserve_Particle, Category = "Ammo") FGameplayAttributeData Reserve_Particle;

protected:
	UFUNCTION() void OnRep_Reserve_Bullet(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_Reserve_Battery(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_Reserve_Grenade(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_Reserve_Radiator(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_Reserve_Particle(const FGameplayAttributeData& OldValue);
};
