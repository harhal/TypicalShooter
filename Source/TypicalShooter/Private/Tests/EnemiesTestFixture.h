// Shared fixture for enemy tests: a playing bare world with a floor, an enemy at the origin facing +X and helpers.

#pragma once

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/BoxComponent.h"
#include "Enemies/EnemyCharacter.h"
#include "Engine/HitResult.h"
#include "AI/NavigationSystemBase.h"
#include "Shooting/ShootingTags.h"
#include "Shooting/ShotDamage.h"
#include "Shooting/ShotDamageReceiver.h"
#include "Shooting/ShotProjectile.h"
#include "Tests/ShootingTestFixture.h"

namespace EnemiesTest
{
	/** Static box floor with its top at Z = 0. */
	inline AActor* SpawnFloor(const ShootingTest::FWorldFixture& F, const FVector& Extent = FVector(5000.f, 5000.f, 10.f))
	{
		AActor* Floor = F.World->SpawnActor<AActor>();
		UBoxComponent* Box = NewObject<UBoxComponent>(Floor);
		Box->SetBoxExtent(Extent);
		Box->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
		Floor->SetRootComponent(Box);
		Box->RegisterComponent();
		Box->SetWorldLocation(FVector(0.f, 0.f, -Extent.Z));
		return Floor;
	}

	/** Navigation system without nav data: AI straight-line moves work (bare worlds have none). */
	inline void AddNavigation(const ShootingTest::FWorldFixture& F)
	{
		FNavigationSystem::AddNavigationSystemToWorld(*F.World, FNavigationSystemRunMode::GameMode);
	}

	/** Enemy standing on the floor at Location, facing Yaw. */
	inline AEnemyCharacter* SpawnEnemy(const ShootingTest::FWorldFixture& F, const FVector& Location = FVector::ZeroVector, const float Yaw = 0.f)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		return F.World->SpawnActor<AEnemyCharacter>(Location + FVector(0.f, 0.f, 100.f), FRotator(0.f, Yaw, 0.f), Params);
	}

	inline void Hit(AActor* Target, const float Amount)
	{
		FShotDamage Damage;
		Damage.Amounts.Add(ShootingTags::Damage_Type_Physical, Amount);
		IShotDamageReceiver::FindOnActor(Target)->ReceiveShotDamage(Damage, FHitResult());
	}

	/** Counts projectiles spawned in the world from now on. */
	inline TSharedRef<int32> CountProjectiles(const ShootingTest::FWorldFixture& F)
	{
		TSharedRef<int32> Counter = MakeShared<int32>(0);
		F.World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateLambda([Counter](AActor* Spawned)
		{
			*Counter += Spawned && Spawned->IsA<AShotProjectile>() ? 1 : 0;
		}));
		return Counter;
	}
}

#endif
