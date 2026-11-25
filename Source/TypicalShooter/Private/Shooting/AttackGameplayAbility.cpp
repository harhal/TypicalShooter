#include "Shooting/AttackGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Shooting/AttackAttributeSet.h"
#include "Shooting/AttackModifiersComponent.h"
#include "Shooting/ReloadGameplayAbility.h"
#include "Shooting/ShootingTags.h"
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
	const UAbilitySystemComponent* Asc = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (Asc && Asc->HasMatchingGameplayTag(ShootingTags::GetReloadingTag(static_cast<uint8>(Slot))))
	{
		return false;
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
		if (!FireShot())
		{
			// An empty clip cuts the burst and starts this slot's reload.
			UAbilitySystemComponent* Asc = GetAbilitySystemComponentFromActorInfo();
			Finish();
			UReloadGameplayAbility::TryReload(*Asc, Slot);
			return;
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

bool UAttackGameplayAbility::FireShot()
{
	UAbilitySystemComponent* Asc = GetAbilitySystemComponentFromActorInfo();
	const FAttackStats Stats = UAttackAttributeSet::ReadStats(*Asc, Slot);
	if (Stats.Ammo <= 0)
	{
		return false;
	}
	Asc->SetNumericAttributeBase(UAttackAttributeSet::GetAttribute(Slot, EAttackAttribute::Ammo), Stats.Ammo - 1);
	SpawnProjectiles(Stats);
	return true;
}

void UAttackGameplayAbility::SpawnProjectiles(const FAttackStats& Stats) const
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	const FShotAim Aim = FShotAim::Compute(*Avatar, Stats.FireEndDistance);
	const FVector AimDirection = ShotMath::AimDirection(Aim.Muzzle, Aim.AimPoint, Aim.ViewDirection);

	FProjectileShotParams Params;
	Params.Damage = Stats.Damage;
	Params.Damage.Instigator = Avatar;
	Params.Speed = Stats.ProjectileSpeed;
	Params.GravityScale = Stats.GravityScale;
	Params.MaxTravelDistance = Stats.FireEndDistance - Stats.FireStartDistance;
	Params.DamageLossPerHit = Stats.DamageLoss;

	APawn* Instigator = Cast<APawn>(Avatar);
	const UAttackModifiersComponent* Modifiers = Avatar->FindComponentByClass<UAttackModifiersComponent>();
	const FShotPattern* Override = Modifiers ? Modifiers->FindPatternOverride(Slot) : nullptr;
	const FShotPattern& ActivePattern = Override ? *Override : Pattern;
	for (const FVector& Direction : ShotMath::ProjectileDirections(AimDirection, ActivePattern, Stats.PatternScale))
	{
		AShotProjectile::SpawnShot(Avatar->GetWorld(), ProjectileClass,
			ShotMath::SpawnTransform(Aim.Muzzle, Direction, Stats.FireStartDistance), Params, Avatar, Instigator);
	}
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
		// Never loop within a frame (zero burst interval + held input would empty the clip at once).
		Timer = Timers.SetTimerForNextTick(FTimerDelegate::CreateUObject(this, Callback));
		return;
	}
	Timers.SetTimer(Timer, FTimerDelegate::CreateUObject(this, Callback), Delay, false);
}
