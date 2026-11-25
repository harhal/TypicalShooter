// Tests for the pure shot math: shot pattern directions, aim from the muzzle, spawn transforms (R5, R6).

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Shooting/ShotPattern.h"
#include "Tests/ShootingTestFixture.h"

namespace ShootingPatternTest
{
	FVector YawDir(const float Degrees)
	{
		return FVector(FMath::Cos(FMath::DegreesToRadians(Degrees)), FMath::Sin(FMath::DegreesToRadians(Degrees)), 0.f);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FShotPatternSingleTest, "TypicalShooter.Shooting.Pattern.SinglePointFollowsAim", ShootingTest::Flags)
bool FShotPatternSingleTest::RunTest(const FString& Parameters)
{
	const FShotPattern Pattern;
	TestEqual(TEXT("Default pattern fires one projectile"), Pattern.GetNumProjectiles(), 1);

	const TArray<FVector> Directions = ShotMath::ProjectileDirections(FVector(0.f, 1.f, 0.f), Pattern, 3.f);
	if (TestEqual(TEXT("One direction"), Directions.Num(), 1))
	{
		TestTrue(TEXT("Centred projectile flies along the aim"), Directions[0].Equals(FVector(0.f, 1.f, 0.f), 1e-4));
	}

	FShotPattern Empty;
	Empty.Offsets.Reset();
	TestEqual(TEXT("Empty pattern still fires one projectile"), ShotMath::ProjectileDirections(FVector::ForwardVector, Empty, 1.f).Num(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FShotPatternOffsetsTest, "TypicalShooter.Shooting.Pattern.OffsetsScaled", ShootingTest::Flags)
bool FShotPatternOffsetsTest::RunTest(const FString& Parameters)
{
	using namespace ShootingPatternTest;
	FShotPattern Pattern;
	Pattern.Offsets = { FVector2D(0.f, 0.f), FVector2D(10.f, 0.f), FVector2D(-10.f, 0.f), FVector2D(0.f, 5.f) };
	TestEqual(TEXT("One projectile per point"), Pattern.GetNumProjectiles(), 4);

	const TArray<FVector> Dirs = ShotMath::ProjectileDirections(FVector::ForwardVector, Pattern, 1.f);
	if (!TestEqual(TEXT("Four directions"), Dirs.Num(), 4))
	{
		return false;
	}
	TestTrue(TEXT("Positive yaw turns right"), Dirs[1].Equals(YawDir(10.f), 1e-4));
	TestTrue(TEXT("Negative yaw turns left"), Dirs[2].Equals(YawDir(-10.f), 1e-4));
	TestTrue(TEXT("Positive pitch goes up"), FMath::IsNearlyEqual(Dirs[3].Z, FMath::Sin(FMath::DegreesToRadians(5.f)), 1e-4));
	TestTrue(TEXT("Directions are unit vectors"), Dirs[3].IsNormalized());

	const TArray<FVector> Doubled = ShotMath::ProjectileDirections(FVector::ForwardVector, Pattern, 2.f);
	TestTrue(TEXT("Scale multiplies the offsets"), Doubled[1].Equals(YawDir(20.f), 1e-4));
	const TArray<FVector> Collapsed = ShotMath::ProjectileDirections(FVector::ForwardVector, Pattern, 0.f);
	TestTrue(TEXT("Zero scale collapses the pattern onto the aim"), Collapsed[2].Equals(FVector::ForwardVector, 1e-4));

	const TArray<FVector> Sideways = ShotMath::ProjectileDirections(FVector(0.f, 1.f, 0.f), Pattern, 1.f);
	TestTrue(TEXT("Offsets are relative to the aim direction"), Sideways[1].Equals(YawDir(100.f), 1e-4));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FShotAimTest, "TypicalShooter.Shooting.Pattern.AimFromMuzzle", ShootingTest::Flags)
bool FShotAimTest::RunTest(const FString& Parameters)
{
	const FVector Muzzle(10.f, 0.f, 0.f);
	TestTrue(TEXT("Aim points from the muzzle to the aim point"),
		ShotMath::AimDirection(Muzzle, FVector(110.f, 0.f, 100.f), FVector::ForwardVector).Equals(FVector(1.f, 0.f, 1.f).GetSafeNormal(), 1e-4));
	TestTrue(TEXT("Coinciding points use the fallback"),
		ShotMath::AimDirection(Muzzle, Muzzle, FVector(0.f, 5.f, 0.f)).Equals(FVector(0.f, 1.f, 0.f), 1e-4));

	const FTransform Spawn = ShotMath::SpawnTransform(Muzzle, FVector(0.f, 1.f, 0.f), 50.f);
	TestTrue(TEXT("Spawned at the start distance from the muzzle"), Spawn.GetLocation().Equals(FVector(10.f, 50.f, 0.f), 1e-3));
	TestTrue(TEXT("Spawn faces the direction"), Spawn.GetRotation().GetForwardVector().Equals(FVector(0.f, 1.f, 0.f), 1e-4));
	return true;
}

#endif
