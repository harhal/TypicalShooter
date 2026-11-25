// Attack controls of a player pawn, built in code (no assets yet): LMB / RT primary, RMB / LT alternate.
// Presses go to the pawn's ASC as ability input ids (EAttackInputID); ignored while the inventory screen is open.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Shooting/AttackTypes.h"
#include "AttackInputComponent.generated.h"

class UAbilitySystemComponent;
class UEnhancedInputComponent;
class UInputAction;
class UInputMappingContext;

UCLASS(ClassGroup = (Shooting))
class TYPICALSHOOTER_API UAttackInputComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAttackInputComponent();

	/** Binds the attack actions and adds their mapping context for the pawn's local player. Call from SetupPlayerInputComponent. */
	void BindInput(UEnhancedInputComponent& Input);

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	int32 MappingPriority = 0;

private:
	void CreateInput();
	void AddMappingContext() const;
	void HandlePressed(EAttackInputID Id);
	void HandleReleased(EAttackInputID Id);
	UAbilitySystemComponent* GetAbilitySystem() const;
	bool IsInventoryOpen() const;

	UPROPERTY(Transient)
	TObjectPtr<UInputMappingContext> MappingContext;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> PrimaryAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> AlternateAction;
};
