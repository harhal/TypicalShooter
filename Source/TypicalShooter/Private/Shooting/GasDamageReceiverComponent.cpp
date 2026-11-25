#include "Shooting/GasDamageReceiverComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Shooting/ShootingTags.h"
#include "Shooting/ShotDamage.h"
#include "Shooting/ShotDamageEffect.h"

UGasDamageReceiverComponent::UGasDamageReceiverComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	DamageEffect = UShotDamageEffect::StaticClass();
}

void UGasDamageReceiverComponent::ReceiveShotDamage(const FShotDamage& Damage, const FHitResult& Hit)
{
	const AActor* Owner = GetOwner();
	UAbilitySystemComponent* Asc = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner);
	if (!Owner || !Owner->HasAuthority() || !Asc || !DamageEffect)
	{
		return;
	}

	FGameplayEffectContextHandle Context = Asc->MakeEffectContext();
	Context.AddInstigator(Damage.Instigator.Get(), Damage.Instigator.Get());
	Context.AddHitResult(Hit);

	const FGameplayEffectSpecHandle Spec = Asc->MakeOutgoingSpec(DamageEffect, 1.f, Context);
	if (!Spec.IsValid())
	{
		return;
	}
	for (const FGameplayTag& Type : ShootingTags::GetDamageTypes())
	{
		Spec.Data->SetSetByCallerMagnitude(Type, FMath::Max(0.f, Damage.Get(Type)));
	}
	Asc->ApplyGameplayEffectSpecToSelf(*Spec.Data);
}
