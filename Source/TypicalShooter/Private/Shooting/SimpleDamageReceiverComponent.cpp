#include "Shooting/SimpleDamageReceiverComponent.h"

#include "GameFramework/Actor.h"
#include "Shooting/ShotDamage.h"

USimpleDamageReceiverComponent::USimpleDamageReceiverComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
}

void USimpleDamageReceiverComponent::InitializeComponent()
{
	Super::InitializeComponent();
	Durability = MaxDurability;
}

void USimpleDamageReceiverComponent::ReceiveShotDamage(const FShotDamage& Damage, const FHitResult& Hit)
{
	AActor* Owner = GetOwner();
	if (IsDepleted() || !Owner || !Owner->HasAuthority())
	{
		return;
	}

	Durability = FMath::Max(0.f, Durability - FMath::Max(0.f, Damage.GetTotal()));
	if (!IsDepleted())
	{
		return;
	}

	DepletedDelegate.Broadcast();
	OnDepleted.Broadcast(Damage.Instigator.Get());
	if (bDestroyOwnerOnDepleted)
	{
		Owner->Destroy();
	}
}
