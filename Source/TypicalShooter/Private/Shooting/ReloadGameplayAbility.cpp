#include "Shooting/ReloadGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "Shooting/AttackAttributeSet.h"
#include "Shooting/ShootingTags.h"
#include "TimerManager.h"

UReloadGameplayAbility::UReloadGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

bool UReloadGameplayAbility::TryReload(UAbilitySystemComponent& Asc, const EAttackSlot Slot)
{
	for (const FGameplayAbilitySpec& Spec : Asc.GetActivatableAbilities())
	{
		const UReloadGameplayAbility* Reload = Cast<UReloadGameplayAbility>(Spec.GetPrimaryInstance());
		Reload = Reload ? Reload : Cast<UReloadGameplayAbility>(Spec.Ability);
		if (Reload && Reload->Slot == Slot)
		{
			return Asc.TryActivateAbility(Spec.Handle);
		}
	}
	return false;
}

void UReloadGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	UAbilitySystemComponent* Asc = GetAbilitySystemComponentFromActorInfo();
	if (!HasAuthority(&ActivationInfo) || !Asc)
	{
		Finish();
		return;
	}

	const FAttackStats Stats = UAttackAttributeSet::ReadStats(*Asc, Slot);
	if (Stats.Ammo >= Stats.ClipSize)
	{
		Finish();
		return;
	}

	Asc->AddLooseGameplayTag(ShootingTags::GetReloadingTag(static_cast<uint8>(Slot)));
	bReloadingTagAdded = true;
	if (Stats.ReloadTime <= 0.f)
	{
		CompleteReload();
		return;
	}
	GetWorld()->GetTimerManager().SetTimer(Timer, FTimerDelegate::CreateUObject(this, &UReloadGameplayAbility::CompleteReload),
		Stats.ReloadTime, false);
}

void UReloadGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const bool bReplicateEndAbility, const bool bWasCancelled)
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(Timer);
	}
	UAbilitySystemComponent* Asc = GetAbilitySystemComponentFromActorInfo();
	if (bReloadingTagAdded && Asc)
	{
		Asc->RemoveLooseGameplayTag(ShootingTags::GetReloadingTag(static_cast<uint8>(Slot)));
	}
	bReloadingTagAdded = false;
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UReloadGameplayAbility::CompleteReload()
{
	if (UAbilitySystemComponent* Asc = GetAbilitySystemComponentFromActorInfo())
	{
		const FAttackStats Stats = UAttackAttributeSet::ReadStats(*Asc, Slot);
		Asc->SetNumericAttributeBase(UAttackAttributeSet::GetAttribute(Slot, EAttackAttribute::Ammo), Stats.ClipSize);
	}
	Finish();
}

void UReloadGameplayAbility::Finish()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}
