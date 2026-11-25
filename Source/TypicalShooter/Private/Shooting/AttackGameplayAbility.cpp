#include "Shooting/AttackGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InventoryItem.h"
#include "PowerGrid/SourceAmmoComponent.h"
#include "Shooting/AttackAttributeSet.h"
#include "Shooting/ShotAim.h"
#include "Shooting/ShotProjectile.h"
#include "TimerManager.h"

UAttackGameplayAbility::UAttackGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	bReplicateInputDirectly = true; // the server checks the held button to repeat bursts
	ProjectileClass = AShotProjectile::StaticClass();
}

bool UAttackGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (Slot == EAttackSlot::Air)
	{
		const ACharacter* Character = ActorInfo ? Cast<ACharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
		const UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;
		if (!Movement || !Movement->IsFalling())
		{
			return false;
		}
	}
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UAttackGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (!HasAuthority(&ActivationInfo) || !GetAbilitySystemComponentFromActorInfo() || !GetAvatarActorFromActorInfo())
	{
		Finish();
		return;
	}
	StartBurst();
}

void UAttackGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const bool bReplicateEndAbility, const bool bWasCancelled)
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(Timer);
	}
	ShotsLeftInBurst = 0;
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UAttackGameplayAbility::StartBurst()
{
	ShotsLeftInBurst = UAttackAttributeSet::ReadStats(*GetAbilitySystemComponentFromActorInfo(), Slot).ShotsPerBurst;
	FireNextShot();
}

void UAttackGameplayAbility::FireNextShot()
{
	const FAttackStats Stats = UAttackAttributeSet::ReadStats(*GetAbilitySystemComponentFromActorInfo(), Slot);
	// Shots with no interval go out in the same frame.
	while (ShotsLeftInBurst > 0)
	{
		FireShot();
		if (!IsActive())
		{
			return; // the shot emptied the sources: the receiver lost power and its ability was taken away
		}
		--ShotsLeftInBurst;
		if (ShotsLeftInBurst > 0 && Stats.ShotInterval > 0.f)
		{
			SetTimer(Stats.ShotInterval, &UAttackGameplayAbility::FireNextShot);
			return;
		}
	}
	SetTimer(Stats.BurstInterval, &UAttackGameplayAbility::OnBurstIntervalElapsed);
}

void UAttackGameplayAbility::OnBurstIntervalElapsed()
{
	const FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec();
	if (Spec && Spec->InputPressed)
	{
		StartBurst();
	}
	else
	{
		Finish();
	}
}

void UAttackGameplayAbility::FireShot()
{
	UAbilitySystemComponent* Asc = GetAbilitySystemComponentFromActorInfo();
	const FAttackStats ShotStats = UAttackAttributeSet::ReadStats(*Asc, Slot);
	const FVector Direction = SpawnProjectiles(ShotStats);
	ApplyRecoil(Direction, ShotStats.Recoil);
	ReportUse();
}

void UAttackGameplayAbility::ReportUse() const
{
	// The receiver item granting this ability is the spec's source object; its sources pay for the shot (R9b).
	const AActor* Avatar = GetAvatarActorFromActorInfo();
	USourceAmmoComponent* SourceAmmo = Avatar ? Avatar->FindComponentByClass<USourceAmmoComponent>() : nullptr;
	if (const UInventoryItem* Receiver = Cast<UInventoryItem>(GetCurrentSourceObject()); SourceAmmo && Receiver)
	{
		SourceAmmo->ReportUse(Receiver);
	}
}

void UAttackGameplayAbility::ApplyRecoil(const FVector& ShotDirection, const float Recoil) const
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character && Recoil > 0.f)
	{
		// Added to the current velocity, once per shot whatever the projectile count (power grid R5c).
		Character->LaunchCharacter(-ShotDirection * Recoil, false, false);
	}
}

FVector UAttackGameplayAbility::SpawnProjectiles(const FAttackStats& Stats) const
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	const FShotAim Aim = FShotAim::Compute(*Avatar, Stats.FireEndDistance);
	// The air attack fires straight down whatever the view (power grid R5).
	const FVector AimDirection = Slot == EAttackSlot::Air
		? FVector::DownVector : ShotMath::AimDirection(Aim.Muzzle, Aim.AimPoint, Aim.ViewDirection);

	FProjectileShotParams Params;
	Params.Damage = Stats.Damage;
	Params.Damage.Instigator = Avatar;
	Params.Speed = Stats.ProjectileSpeed;
	Params.GravityScale = Stats.GravityScale;
	Params.MaxTravelDistance = Stats.FireEndDistance - Stats.FireStartDistance;
	Params.DamageLossPerHit = Stats.DamageLoss;

	APawn* Instigator = Cast<APawn>(Avatar);
	for (const FVector& Direction : ShotMath::ProjectileDirections(AimDirection, Pattern, Stats.PatternScale))
	{
		AShotProjectile::SpawnShot(Avatar->GetWorld(), ProjectileClass,
			ShotMath::SpawnTransform(Aim.Muzzle, Direction, Stats.FireStartDistance), Params, Avatar, Instigator);
	}
	return AimDirection;
}

void UAttackGameplayAbility::Finish()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UAttackGameplayAbility::SetTimer(const float Delay, void (UAttackGameplayAbility::*Callback)())
{
	FTimerManager& Timers = GetWorld()->GetTimerManager();
	if (Delay <= 0.f)
	{
		// Never loop within a frame (zero burst interval + held input would fire endlessly in one frame).
		Timer = Timers.SetTimerForNextTick(FTimerDelegate::CreateUObject(this, Callback));
		return;
	}
	Timers.SetTimer(Timer, FTimerDelegate::CreateUObject(this, Callback), Delay, false);
}
