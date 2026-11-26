#include "Enemies/EnemyCharacter.h"

#include "TS_GAS.h"
#include "Components/CapsuleComponent.h"
#include "Shooting/AttackAttributeSet.h"
#include "Shooting/DefaultAttackSetup.h"
#include "Shooting/GasDamageReceiverComponent.h"
#include "Shooting/HealthAttributeSet.h"
#include "Shooting/HealthComponent.h"
#include "Shooting/ShotAim.h"

AEnemyCharacter::AEnemyCharacter()
{
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
	if (bFire == bFiring || !HasAuthority())
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
	}
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
