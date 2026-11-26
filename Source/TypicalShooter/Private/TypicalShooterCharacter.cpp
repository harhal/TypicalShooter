// Copyright Epic Games, Inc. All Rights Reserved.

#include "TypicalShooterCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "InteractionComponent.h"
#include "InventoryComponent.h"
#include "Movement/JumpGameplayAbility.h"
#include "Movement/MovementAttributeSet.h"
#include "Movement/MovementAttributesComponent.h"
#include "PowerGrid/PowerEffectsComponent.h"
#include "PowerGrid/PowerGridComponent.h"
#include "PowerGrid/ReceiverAbilitiesComponent.h"
#include "PowerGrid/SourceAmmoComponent.h"
#include "TS_GAS.h"
#include "Shooting/AmmoAttributeSet.h"
#include "Shooting/AttackAttributeSet.h"
#include "Shooting/AttackInputComponent.h"
#include "Shooting/DefaultAttackSetup.h"
#include "Shooting/GasDamageReceiverComponent.h"
#include "Shooting/HealthAttributeSet.h"
#include "Shooting/HealthComponent.h"
#include "Shooting/ShotAim.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TypicalShooter.h"

ATypicalShooterCharacter::ATypicalShooterCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	
	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;
	
	Gas = CreateDefaultSubobject<UTS_GAS>(TEXT("Ability System"));
	Gas->SetIsReplicated(true);
	Gas->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	AttackAttributes = CreateDefaultSubobject<UAttackAttributeSet>(TEXT("Attack Attributes"));
	AmmoAttributes = CreateDefaultSubobject<UAmmoAttributeSet>(TEXT("Ammo Attributes"));
	HealthAttributes = CreateDefaultSubobject<UHealthAttributeSet>(TEXT("Health Attributes"));
	Health = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
	Health->bRefillWhenDepleted = true;
	DamageReceiver = CreateDefaultSubobject<UGasDamageReceiverComponent>(TEXT("Damage Receiver"));
	MovementAttributeSet = CreateDefaultSubobject<UMovementAttributeSet>(TEXT("Movement Attribute Set"));
	MovementAttributes = CreateDefaultSubobject<UMovementAttributesComponent>(TEXT("Movement Attributes"));
	JumpAbility = UJumpGameplayAbility::StaticClass();

	Muzzle = CreateDefaultSubobject<USceneComponent>(TEXT("Muzzle"));
	Muzzle->SetupAttachment(FirstPersonCameraComponent);
	Muzzle->SetRelativeLocation(FVector(40.f, 15.f, -12.f));
	Muzzle->ComponentTags.Add(FShotAim::MuzzleTag);

	AttackInput = CreateDefaultSubobject<UAttackInputComponent>(TEXT("Attack Input"));
	AttackDefaultsEffect = UDefaultAttackStatsEffect::StaticClass();

	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("Interaction Component"));

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory Component"));
	PowerGrid = CreateDefaultSubobject<UPowerGridComponent>(TEXT("Power Grid"));
	ReceiverAbilities = CreateDefaultSubobject<UReceiverAbilitiesComponent>(TEXT("Receiver Abilities"));
	PowerEffects = CreateDefaultSubobject<UPowerEffectsComponent>(TEXT("Power Effects"));
	SourceAmmo = CreateDefaultSubobject<USourceAmmoComponent>(TEXT("Source Ammo"));
	StartingItems = DefaultLoadout::MakeStartingItems();
}

void ATypicalShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ATypicalShooterCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ATypicalShooterCharacter::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATypicalShooterCharacter::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATypicalShooterCharacter::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ATypicalShooterCharacter::LookInput);

		// Attacks (actions and mapping context are generated by the component)
		AttackInput->BindInput(*EnhancedInputComponent);

		UE_LOG(LogTypicalShooter, Verbose, TEXT("%s: bound %d input actions"), *GetNameSafe(this), EnhancedInputComponent->GetActionEventBindings().Num());
	}
	else
	{
		UE_LOG(LogTypicalShooter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

UAbilitySystemComponent* ATypicalShooterCharacter::GetAbilitySystemComponent() const
{
	return Gas;
}

void ATypicalShooterCharacter::BeginPlay()
{
	// Before the components' BeginPlay: the power grid components apply abilities and item effects to the ASC there.
	Gas->InitAbilityActorInfo(this, this);
	Super::BeginPlay();
	if (HasAuthority())
	{
		GrantStartingLoadout();
	}
}

void ATypicalShooterCharacter::GrantStartingLoadout()
{
	if (JumpAbility)
	{
		Gas->GiveAbility(FGameplayAbilitySpec(JumpAbility, 1, static_cast<int32>(EAttackInputID::Jump), this));
	}
	if (AttackDefaultsEffect)
	{
		Gas->ApplyGameplayEffectToSelf(AttackDefaultsEffect.GetDefaultObject(), 1.f, Gas->MakeEffectContext());
	}
	// Receivers among the starting items grant the attacks through ReceiverAbilities (power grid R6a).
	for (const FStartingInventoryItem& Item : StartingItems)
	{
		if (!InventoryComponent->AddItem(Item.Manifest, Item.Location))
		{
			UE_LOG(LogTypicalShooter, Warning, TEXT("%s: starting item does not fit at (%d,%d)"),
				*GetName(), Item.Location.CellCoord.X, Item.Location.CellCoord.Y);
		}
	}
	UE_LOG(LogTypicalShooter, Verbose, TEXT("%s: starting loadout of %d items"), *GetName(), StartingItems.Num());
}


void ATypicalShooterCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);

}

void ATypicalShooterCharacter::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);

}

void ATypicalShooterCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// pass the rotation inputs
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ATypicalShooterCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void ATypicalShooterCharacter::DoJumpStart()
{
	// The jump is a GAS ability (power grid R6c): the press activates it, the release ends it.
	Gas->AbilityLocalInputPressed(static_cast<int32>(EAttackInputID::Jump));
}

void ATypicalShooterCharacter::DoJumpEnd()
{
	Gas->AbilityLocalInputReleased(static_cast<int32>(EAttackInputID::Jump));
}
