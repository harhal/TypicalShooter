// Shared fixture for attack ability tests: a shooter actor with an ASC, attack attributes and the attack ability of
// one slot, plus a sturdy target dummy in front of it.

#pragma once

#if WITH_DEV_AUTOMATION_TESTS

#include "AbilitySystemComponent.h"
#include "Shooting/AmmoAttributeSet.h"
#include "Shooting/AttackAttributeSet.h"
#include "Shooting/AttackGameplayAbility.h"
#include "Shooting/HealthAttributeSet.h"
#include "Shooting/ShotProjectile.h"
#include "Shooting/TargetDummy.h"
#include "Tests/ShootingTestFixture.h"

namespace ShootingAbilityTest
{
	/** Actor at the origin looking along +X, with attack attributes and an attack ability of Slot. */
	struct FShooter
	{
		UAbilitySystemComponent* Asc = nullptr;
		FGameplayAbilitySpecHandle Handle;
		/** Projectiles spawned in the world since the shooter was created. */
		TSharedRef<int32> NumProjectiles = MakeShared<int32>(0);

		FShooter(const ShootingTest::FWorldFixture& F, const EAttackSlot Slot, const FShotPattern& Pattern = FShotPattern())
		{
			Asc = F.SpawnWithComponent<UAbilitySystemComponent>();
			AActor* Actor = Asc->GetOwner();
			Asc->InitAbilityActorInfo(Actor, Actor);
			Asc->AddSpawnedAttribute(NewObject<UAttackAttributeSet>(Actor));
			Asc->AddSpawnedAttribute(NewObject<UAmmoAttributeSet>(Actor));

			Handle = Asc->GiveAbility(FGameplayAbilitySpec(UAttackGameplayAbility::StaticClass()));
			UAttackGameplayAbility* Ability = Cast<UAttackGameplayAbility>(Spec()->GetPrimaryInstance());
			Ability->Slot = Slot;
			Ability->Pattern = Pattern;

			F.World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateLambda([Counter = NumProjectiles](AActor* Spawned)
			{
				*Counter += Spawned && Spawned->IsA<AShotProjectile>() ? 1 : 0;
			}));
		}

		FGameplayAbilitySpec* Spec() const { return Asc->FindAbilitySpecFromHandle(Handle); }
		bool Activate() const { return Asc->TryActivateAbility(Handle); }
		bool IsActive() const { return Spec()->IsActive(); }
		void SetHeld(const bool bHeld) const { Spec()->InputPressed = bHeld; }
		int32 Projectiles() const { return *NumProjectiles; }

		void SetReserve(const FGameplayTag& AmmoType, const float Value) const
		{
			Asc->SetNumericAttributeBase(UAmmoAttributeSet::GetReserveAttribute(AmmoType), Value);
		}

		float Reserve(const FGameplayTag& AmmoType) const
		{
			return Asc->GetNumericAttribute(UAmmoAttributeSet::GetReserveAttribute(AmmoType));
		}

		void Set(const EAttackSlot Slot, const EAttackAttribute Attribute, const float Value) const
		{
			Asc->SetNumericAttributeBase(UAttackAttributeSet::GetAttribute(Slot, Attribute), Value);
		}

		float Get(const EAttackSlot Slot, const EAttackAttribute Attribute) const
		{
			return Asc->GetNumericAttribute(UAttackAttributeSet::GetAttribute(Slot, Attribute));
		}

		/** Shots per burst, intervals, physical damage per hit. */
		void Setup(const EAttackSlot Slot, const float Shots, const float ShotInterval, const float BurstInterval, const float Damage) const
		{
			Set(Slot, EAttackAttribute::ShotsPerBurst, Shots);
			Set(Slot, EAttackAttribute::ShotInterval, ShotInterval);
			Set(Slot, EAttackAttribute::BurstInterval, BurstInterval);
			Set(Slot, EAttackAttribute::DamagePhysical, Damage);
			Set(Slot, EAttackAttribute::ProjectileSpeed, 10000.f);
		}
	};

	/** Dummy with 1000 health at X = 500. */
	inline ATargetDummy* SpawnDummy(const ShootingTest::FWorldFixture& F)
	{
		ATargetDummy* Dummy = F.World->SpawnActor<ATargetDummy>(FVector(500.f, 0.f, 0.f), FRotator::ZeroRotator);
		Dummy->MaxHealth = 1000.f; // BeginPlay already ran: set the attributes too
		Dummy->GetAbilitySystemComponent()->SetNumericAttributeBase(UHealthAttributeSet::GetMaxHealthAttribute(), 1000.f);
		Dummy->GetAbilitySystemComponent()->SetNumericAttributeBase(UHealthAttributeSet::GetHealthAttribute(), 1000.f);
		return Dummy;
	}

	inline float Lost(const ATargetDummy* Dummy)
	{
		return Dummy->GetTotalDamageTaken();
	}
}

#endif
