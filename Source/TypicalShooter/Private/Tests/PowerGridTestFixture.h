// Shared fixture for power grid receiver tests: an AI character on a floor with an ASC, attack attributes,
// an inventory, a power grid and receiver abilities, in a ticking game world (no PIE).

#pragma once

#if WITH_DEV_AUTOMATION_TESTS

#include "AbilitySystemComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "InventoryComponent.h"
#include "PowerGrid/PowerGridComponent.h"
#include "PowerGrid/PowerGridFragment.h"
#include "PowerGrid/ReceiverAbilitiesComponent.h"
#include "Shooting/AttackAttributeSet.h"
#include "Shooting/AttackGameplayAbility.h"
#include "Shooting/ShotProjectile.h"
#include "Tests/ShootingTestFixture.h"

namespace PowerGridTest
{
	/** Power item of Type; receivers use the plain attack ability. */
	inline FInventoryItemManifest Manifest(const EPowerItemType Type, const TArray<FAttackStatBonus>& BaseStats = {})
	{
		FPowerGrid_ItemFragment Fragment;
		Fragment.ItemType = Type;
		Fragment.Receiver.Ability = UAttackGameplayAbility::StaticClass();
		Fragment.Receiver.BaseStats = BaseStats;
		FInventoryItemManifest Result;
		Result.Fragments.Add(TInstancedStruct<FItemFragment>::Make<FPowerGrid_ItemFragment>(Fragment));
		return Result;
	}

	/** AI character on a floor with an ASC, attack attributes, inventory, power grid and receiver abilities. */
	struct FArmed
	{
		ACharacter* Character = nullptr;
		UAbilitySystemComponent* Asc = nullptr;
		UInventoryComponent* Inventory = nullptr;
		TArray<TWeakObjectPtr<AShotProjectile>> Projectiles;

		explicit FArmed(const ShootingTest::FWorldFixture& F)
		{
			AActor* Floor = F.World->SpawnActor<AActor>();
			UBoxComponent* Box = NewObject<UBoxComponent>(Floor);
			Box->SetBoxExtent(FVector(2000.f, 2000.f, 10.f));
			Box->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
			Floor->SetRootComponent(Box);
			Box->RegisterComponent();
			Box->SetWorldLocation(FVector(0.f, 0.f, -10.f));

			Character = F.World->SpawnActor<ACharacter>(FVector(0.f, 0.f, 150.f), FRotator::ZeroRotator);
			Character->SpawnDefaultController();
			Asc = NewObject<UAbilitySystemComponent>(Character);
			Asc->RegisterComponent();
			Asc->InitAbilityActorInfo(Character, Character);
			Asc->AddSpawnedAttribute(NewObject<UAttackAttributeSet>(Character));
			Inventory = NewObject<UInventoryComponent>(Character);
			Inventory->RegisterComponent();
			NewObject<UPowerGridComponent>(Character)->RegisterComponent();
			NewObject<UReceiverAbilitiesComponent>(Character)->RegisterComponent();

			F.World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateLambda([this](AActor* Spawned)
			{
				if (AShotProjectile* Projectile = Cast<AShotProjectile>(Spawned))
				{
					Projectiles.Add(Projectile);
				}
			}));
		}

		/** Spec granted with Input, nullptr if none. */
		const FGameplayAbilitySpec* FindSpec(const EAttackInputID Input) const
		{
			for (const FGameplayAbilitySpec& Spec : Asc->GetActivatableAbilities())
			{
				if (Spec.InputID == static_cast<int32>(Input))
				{
					return &Spec;
				}
			}
			return nullptr;
		}

		TOptional<EAttackSlot> SlotOf(const FGameplayAbilitySpec* Spec) const
		{
			const UAttackGameplayAbility* Ability = Spec ? Cast<UAttackGameplayAbility>(Spec->GetPrimaryInstance()) : nullptr;
			return Ability ? Ability->Slot : TOptional<EAttackSlot>();
		}

		float BaseOf(const EAttackSlot Slot, const EAttackAttribute Attribute) const
		{
			return Asc->GetNumericAttributeBase(UAttackAttributeSet::GetAttribute(Slot, Attribute));
		}
	};
}

#endif
