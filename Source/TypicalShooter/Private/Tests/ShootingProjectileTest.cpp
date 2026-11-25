// Tests for AShotProjectile: projectiles fired at target dummies in a ticking game world (no PIE).

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/BoxComponent.h"
#include "Shooting/ShootingTags.h"
#include "Shooting/ShotProjectile.h"
#include "Shooting/TargetDummy.h"
#include "Tests/ShootingTestFixture.h"

namespace ShootingProjectileTest
{
	FProjectileShotParams MakeParams(const float Physical, const float DamageLoss)
	{
		FProjectileShotParams Params;
		Params.Damage.Amounts.Add(ShootingTags::Damage_Type_Physical, Physical);
		Params.Speed = 5000.f;
		Params.DamageLossPerHit = DamageLoss;
		return Params;
	}

	ATargetDummy* SpawnDummy(const ShootingTest::FWorldFixture& F, const float X)
	{
		return F.World->SpawnActor<ATargetDummy>(FVector(X, 0.f, 0.f), FRotator::ZeroRotator);
	}

	/** Static blocking wall without a damage receiver, across the X axis. */
	AActor* SpawnWall(const ShootingTest::FWorldFixture& F, const float X)
	{
		AActor* Wall = F.World->SpawnActor<AActor>();
		UBoxComponent* Box = NewObject<UBoxComponent>(Wall);
		Box->SetBoxExtent(FVector(10.f, 300.f, 300.f));
		Box->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
		Wall->SetRootComponent(Box);
		Box->RegisterComponent();
		Box->SetWorldLocation(FVector(X, 0.f, 0.f));
		return Wall;
	}

	/** Projectile at the origin flying along +X. */
	AShotProjectile* Fire(const ShootingTest::FWorldFixture& F, const FProjectileShotParams& Params, AActor* Owner = nullptr)
	{
		return AShotProjectile::SpawnShot(F.World, AShotProjectile::StaticClass(), FTransform::Identity, Params, Owner);
	}

	float Lost(const ATargetDummy* Dummy)
	{
		return Dummy->MaxHealth - Dummy->GetHealth();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FProjectileDamagesDummyTest, "TypicalShooter.Shooting.Projectile.DamagesDummy", ShootingTest::Flags)
bool FProjectileDamagesDummyTest::RunTest(const FString& Parameters)
{
	using namespace ShootingProjectileTest;
	const ShootingTest::FWorldFixture F(true);
	ATargetDummy* Shooter = SpawnDummy(F, 0.f); // owner standing around the muzzle is not hit
	ATargetDummy* Target = SpawnDummy(F, 400.f);

	const TWeakObjectPtr<AShotProjectile> Projectile = Fire(F, MakeParams(25.f, 1.f), Shooter);
	if (!TestTrue(TEXT("Projectile spawned"), Projectile.IsValid()))
	{
		return false;
	}
	F.Tick(0.3f);

	TestEqual(TEXT("Target takes the projectile damage"), Lost(Target), 25.f);
	TestEqual(TEXT("Owner is not hit"), Lost(Shooter), 0.f);
	TestFalse(TEXT("Projectile with full damage loss stops at the first hit"), Projectile.IsValid());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FProjectilePierceTest, "TypicalShooter.Shooting.Projectile.PiercesWithDamageLoss", ShootingTest::Flags)
bool FProjectilePierceTest::RunTest(const FString& Parameters)
{
	using namespace ShootingProjectileTest;
	const ShootingTest::FWorldFixture F(true);
	ATargetDummy* First = SpawnDummy(F, 300.f);
	ATargetDummy* Second = SpawnDummy(F, 600.f);

	const TWeakObjectPtr<AShotProjectile> Projectile = Fire(F, MakeParams(40.f, 0.5f));
	F.Tick(0.3f);

	TestEqual(TEXT("First target takes full damage"), Lost(First), 40.f);
	TestEqual(TEXT("Second target takes damage reduced by the loss"), Lost(Second), 20.f);
	if (TestTrue(TEXT("Projectile keeps flying while it has damage"), Projectile.IsValid()))
	{
		TestEqual(TEXT("Remaining damage after two hits"), Projectile->GetRemainingDamage().GetTotal(), 10.f);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FProjectileObstacleTest, "TypicalShooter.Shooting.Projectile.ObstacleReducesDamage", ShootingTest::Flags)
bool FProjectileObstacleTest::RunTest(const FString& Parameters)
{
	using namespace ShootingProjectileTest;
	const ShootingTest::FWorldFixture F(true);
	SpawnWall(F, 300.f);
	ATargetDummy* Target = SpawnDummy(F, 600.f);

	Fire(F, MakeParams(40.f, 0.5f));
	F.Tick(0.3f);
	TestEqual(TEXT("Target behind a wall takes reduced damage"), Lost(Target), 20.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FProjectileObstacleStopsTest, "TypicalShooter.Shooting.Projectile.ObstacleStopsWithFullLoss", ShootingTest::Flags)
bool FProjectileObstacleStopsTest::RunTest(const FString& Parameters)
{
	using namespace ShootingProjectileTest;
	const ShootingTest::FWorldFixture F(true);
	SpawnWall(F, 300.f);
	ATargetDummy* Target = SpawnDummy(F, 600.f);

	const TWeakObjectPtr<AShotProjectile> Projectile = Fire(F, MakeParams(40.f, 1.f));
	F.Tick(0.3f);
	TestEqual(TEXT("Target behind a wall is not hit"), Lost(Target), 0.f);
	TestFalse(TEXT("Projectile destroyed at the wall"), Projectile.IsValid());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FProjectileMaxDistanceTest, "TypicalShooter.Shooting.Projectile.ExpiresAfterMaxDistance", ShootingTest::Flags)
bool FProjectileMaxDistanceTest::RunTest(const FString& Parameters)
{
	using namespace ShootingProjectileTest;
	const ShootingTest::FWorldFixture F(true);
	ATargetDummy* Target = SpawnDummy(F, 500.f);

	FProjectileShotParams Params = MakeParams(25.f, 1.f);
	Params.MaxTravelDistance = 200.f;
	const TWeakObjectPtr<AShotProjectile> Projectile = Fire(F, Params);

	F.Tick(1.f / 60.f);
	TestTrue(TEXT("Alive before reaching the distance"), Projectile.IsValid());
	F.Tick(0.3f);
	TestFalse(TEXT("Destroyed after the max travel distance"), Projectile.IsValid());
	TestEqual(TEXT("Target beyond the distance is not hit"), Lost(Target), 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FProjectileGravityTest, "TypicalShooter.Shooting.Projectile.GravityScale", ShootingTest::Flags)
bool FProjectileGravityTest::RunTest(const FString& Parameters)
{
	using namespace ShootingProjectileTest;
	const ShootingTest::FWorldFixture F(true);

	FProjectileShotParams Straight = MakeParams(1.f, 1.f);
	Straight.Speed = 1000.f;
	FProjectileShotParams Falling = Straight;
	Falling.GravityScale = 1.f;

	const AShotProjectile* StraightShot = Fire(F, Straight);
	const AShotProjectile* FallingShot = Fire(F, Falling);
	F.Tick(0.5f);

	if (TestNotNull(TEXT("Straight projectile alive"), StraightShot) && TestNotNull(TEXT("Falling projectile alive"), FallingShot))
	{
		TestEqual(TEXT("Zero gravity flies straight"), StraightShot->GetActorLocation().Z, 0.0, 0.01);
		TestTrue(TEXT("Gravity pulls the projectile down"), FallingShot->GetActorLocation().Z < -50.0);
		TestTrue(TEXT("Projectiles fly forward"), StraightShot->GetActorLocation().X > 400.0);
	}
	return true;
}

#endif
