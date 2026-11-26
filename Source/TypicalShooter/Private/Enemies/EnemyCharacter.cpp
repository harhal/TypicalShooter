#include "Enemies/EnemyCharacter.h"

#include "TS_GAS.h"
#include "Components/CapsuleComponent.h"
#include "Enemies/EnemyAIController.h"
#include "Enemies/EnemyDirectorSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "TypicalShooter.h"
#include "Shooting/AttackAttributeSet.h"
#include "Shooting/DefaultAttackSetup.h"
#include "Shooting/GasDamageReceiverComponent.h"
#include "Shooting/HealthAttributeSet.h"
#include "Shooting/HealthComponent.h"
#include "Shooting/ShotAim.h"

AEnemyCharacter::AEnemyCharacter()
{
	// Block Visibility (the Pawn profile ignores it): aim traces stop on enemies and enemies block each other's line of sight.
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	Gas = CreateDefaultSubobject<UTS_GAS>(TEXT("Ability System"));
	Gas->SetIsReplicated(true);
	Gas->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	HealthAttributes = CreateDefaultSubobject<UHealthAttributeSet>(TEXT("Health Attributes"));
	AttackAttributes = CreateDefaultSubobject<UAttackAttributeSet>(TEXT("Attack Attributes"));

	Health = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
	Health->MaxHealth = 60.f;
	DamageReceiver = CreateDefaultSubobject<UGasDamageReceiverComponent>(TEXT("Damage Receiver"));

	Muzzle = CreateDefaultSubobject<USceneComponent>(TEXT("Muzzle"));
	Muzzle->SetupAttachment(GetCapsuleComponent());
	Muzzle->SetRelativeLocation(FVector(50.f, 0.f, 50.f));
	Muzzle->ComponentTags.Add(FShotAim::MuzzleTag);

	AIControllerClass = AEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	AttackAbility = UPrimaryAttackAbility::StaticClass();
	using A = EAttackAttribute;
	AttackStats = {
		{ A::FireEndDistance, 5000.f }, { A::ShotsPerBurst, 3.f }, { A::ShotInterval, 0.15f }, { A::BurstInterval, 1.5f },
		{ A::ProjectileSpeed, 2500.f }, { A::DamagePhysical, 4.f } };
}

UAbilitySystemComponent* AEnemyCharacter::GetAbilitySystemComponent() const
{
	return Gas;
}

void AEnemyCharacter::SetFiring(const bool bFire)
{
	if (bFire == bFiring || bDead || !HasAuthority())
	{
		return;
	}
	bFiring = bFire;
	constexpr int32 InputID = static_cast<int32>(EAttackInputID::PrimaryAttack);
	if (bFire)
	{
		Gas->AbilityLocalInputPressed(InputID);
	}
	else
	{
		Gas->AbilityLocalInputReleased(InputID);
	}
}

void AEnemyCharacter::BeginPlay()
{
	// Before the components' BeginPlay: the health component reads the ASC there.
	Gas->InitAbilityActorInfo(this, this);
	Super::BeginPlay();
	if (HasAuthority())
	{
		GrantAttack();
		Health->GetDepletedDelegate().AddUObject(this, &AEnemyCharacter::Die);
		if (UEnemyDirectorSubsystem* Director = GetWorld()->GetSubsystem<UEnemyDirectorSubsystem>())
		{
			Director->Register(this);
		}
	}
}

void AEnemyCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UEnemyDirectorSubsystem* Director = GetWorld()->GetSubsystem<UEnemyDirectorSubsystem>())
	{
		Director->Unregister(this);
	}
	Super::EndPlay(EndPlayReason);
}

bool AEnemyCharacter::CanShootAt(const AActor* Target) const
{
	if (!Target || bDead)
	{
		return false;
	}
	const FVector From = Muzzle->GetComponentLocation();
	const FVector To = Target->GetActorLocation();
	const float Range = Gas->GetNumericAttribute(UAttackAttributeSet::GetAttribute(EAttackSlot::Primary, EAttackAttribute::FireEndDistance));
	if (FVector::DistSquared(From, To) > FMath::Square(Range))
	{
		return false;
	}

	FCollisionQueryParams Params(SCENE_QUERY_STAT(EnemyLineOfSight), false, this);
	FHitResult Hit;
	return !GetWorld()->LineTraceSingleByChannel(Hit, From, To, ECC_Visibility, Params) || Hit.GetActor() == Target;
}

void AEnemyCharacter::UpdateFiring(const AActor* Target)
{
	SetFiring(CanShootAt(Target));
}

void AEnemyCharacter::Die()
{
	if (bDead)
	{
		return;
	}
	SetFiring(false);
	bDead = true;
	Gas->CancelAllAbilities();

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (AController* OwnController = GetController())
	{
		OwnController->StopMovement();
	}
	DetachFromControllerPendingDestroy();
	SetLifeSpan(DeathLifeSpan);
	if (UEnemyDirectorSubsystem* Director = GetWorld()->GetSubsystem<UEnemyDirectorSubsystem>())
	{
		Director->Unregister(this);
	}

	UE_LOG(LogTypicalShooter, Log, TEXT("Enemy %s died"), *GetName());
	DiedDelegate.Broadcast(this);
	OnDied.Broadcast(this);
}

void AEnemyCharacter::GrantAttack()
{
	for (const FAttackStatBonus& Stat : AttackStats)
	{
		Gas->SetNumericAttributeBase(UAttackAttributeSet::GetAttribute(EAttackSlot::Primary, Stat.Attribute), Stat.Value);
	}
	if (AttackAbility)
	{
		Gas->GiveAbility(FGameplayAbilitySpec(AttackAbility, 1, static_cast<int32>(EAttackInputID::PrimaryAttack), this));
	}
}
