// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Shooting/AttackTypes.h"
#include "TypicalShooterCharacter.generated.h"

class UAmmoAttributeSet;
class UAttackAttributeSet;
class UAttackInputComponent;
class UAttackModifiersComponent;
class UGameplayAbility;
class UGameplayEffect;
class UMovementAttributeSet;
class UMovementAttributesComponent;
class UInteractionComponent;
class UInventoryComponent;
class UPowerGridComponent;
class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A basic first person character
 */
UCLASS(abstract)
class ATypicalShooterCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

	/** Pawn mesh: first person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* MouseLookAction;

	UPROPERTY(EditAnywhere, Category ="GAS")
	UAbilitySystemComponent* Gas;

	UPROPERTY(EditAnywhere, Category ="User Interaction")
	TObjectPtr<UInteractionComponent> InteractionComponent;

	/** The player's own items. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category ="Inventory", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInventoryComponent> InventoryComponent;

	/** Power grid of InventoryComponent's items. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category ="Inventory", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPowerGridComponent> PowerGrid;

	/** Projectiles start here (tagged "Muzzle"); follows the first person camera. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category ="Attack", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> Muzzle;

	UPROPERTY(VisibleAnywhere, Category ="Attack")
	TObjectPtr<UAttackInputComponent> AttackInput;

	/** Applies attack modifiers of the items in InventoryComponent. */
	UPROPERTY(VisibleAnywhere, Category ="Attack")
	TObjectPtr<UAttackModifiersComponent> AttackModifiers;

	UPROPERTY()
	TObjectPtr<UAttackAttributeSet> AttackAttributes;

	/** Reserve ammo per kind. */
	UPROPERTY()
	TObjectPtr<UAmmoAttributeSet> AmmoAttributes;

	/** Jump power / air control (power grid R6c), mirrored into the movement component by MovementAttributes. */
	UPROPERTY()
	TObjectPtr<UMovementAttributeSet> MovementAttributeSet;

	UPROPERTY(VisibleAnywhere, Category ="Movement")
	TObjectPtr<UMovementAttributesComponent> MovementAttributes;

	/** Ground jump, granted on the server at BeginPlay and bound to the jump button (EAttackInputID::Jump). */
	UPROPERTY(EditDefaultsOnly, Category ="Movement")
	TSubclassOf<UGameplayAbility> JumpAbility;

	/** Attack abilities granted on the server at BeginPlay. */
	UPROPERTY(EditDefaultsOnly, Category ="Attack")
	TArray<FAttackAbilityGrant> AttackAbilities;

	/** Applied once on the server at BeginPlay: base values of the attack attributes. */
	UPROPERTY(EditDefaultsOnly, Category ="Attack")
	TSubclassOf<UGameplayEffect> AttackDefaultsEffect;

public:
	ATypicalShooterCharacter();

protected:
	virtual void BeginPlay() override;

	/** Server: grants AttackAbilities and JumpAbility, applies AttackDefaultsEffect. */
	void GrantAttacks();

	/** Called from Input Actions for movement input */
	void MoveInput(const FInputActionValue& Value);

	/** Called from Input Actions for looking input */
	void LookInput(const FInputActionValue& Value);

	/** Handles aim inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoAim(float Yaw, float Pitch);

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles jump start inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump end inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

public:

	/** Returns the first person mesh **/
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

	/** Returns first person camera component **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UInteractionComponent* GetInteractionComponent() const { return InteractionComponent; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

	/** Set up input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
};

