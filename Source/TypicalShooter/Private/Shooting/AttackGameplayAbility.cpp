#include "Shooting/AttackGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Shooting/AmmoAttributeSet.h"
#include "Shooting/AttackAttributeSet.h"
#include "Shooting/AttackModifiersComponent.h"
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
	FAttackStats ShotStats = UAttackAttributeSet::ReadStats(*Asc, Slot);
	ApplyShotModifiers(*Asc, ShotStats);
	SpawnProjectiles(ShotStats);
}

void UAttackGameplayAbility::ApplyShotModifiers(UAbilitySystemComponent& Asc, FAttackStats& Stats) const
{
	const UAttackModifiersComponent* Modifiers = GetAvatarActorFromActorInfo()->FindComponentByClass<UAttackModifiersComponent>();
	if (!Modifiers)
	{
		return;
	}
	for (const FAmmoGatedShotModifier& Modifier : Modifiers->GetShotModifiers(Slot))
	{
		const FGameplayAttribute Reserve = UAmmoAttributeSet::GetReserveAttribute(Modifier.AmmoType);
		const float Cost = FMath::Max(Modifier.CostPerShot, 1);
		if (!Reserve.IsValid() || !Asc.HasAttributeSetForAttribute(Reserve) || Asc.GetNumericAttribute(Reserve) < Cost)
		{
			continue; // unpaid: this modifier sits out the shot
		}
		Asc.SetNumericAttributeBase(Reserve, Asc.GetNumericAttribute(Reserve) - Cost);
		for (const FAttackStatBonus& Bonus : Modifier.Bonuses)
		{
			UAttackAttributeSet::AddToStats(Stats, Bonus.Attribute, Bonus.Value);
		}
	}
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
		// Never loop within a frame (zero burst interval + held input would fire endlessly in one frame).
		Timer = Timers.SetTimerForNextTick(FTimerDelegate::CreateUObject(this, Callback));
		return;
	}
	Timers.SetTimer(Timer, FTimerDelegate::CreateUObject(this, Callback), Delay, false);
}
