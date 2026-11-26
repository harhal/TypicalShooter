// Tests for rolled power grid numbers of generated items (LootGeneration R1, R2).

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Abilities/GameplayAbility.h"
#include "LootTestFixture.h"
#include "Loot/LootGenerator.h"
#include "Shooting/ShootingTags.h"

namespace LootPowerParamsTest
{
	/** Level whose default multiplier is exactly 2 (+5% per level from level 1). */
	constexpr int32 DoubleLevel = 21;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootPowerParamsSourceTest, "TypicalShooter.Loot.Generator.PowerParams.Source", LootTestFixture::Flags)
bool FLootPowerParamsSourceTest::RunTest(const FString& Parameters)
{
	using namespace LootTestFixture;
	ULootItemTemplate* Template = MakeTemplate(EPowerItemType::Source, {}, false);
	Template->Source.Power = FLootIntRange(3, 3, true);
	Template->Source.AmmoType = ShootingTags::Ammo_Bullet;
	Template->Source.Capacity = FLootIntRange(10, 20);
	Template->Source.CostPerUse = FLootIntRange(2, 2);
	Template->Source.RechargeDelay = FLootFloatRange(0.5f, 0.5f);

	for (int32 Seed = 0; Seed < 20; ++Seed)
	{
		const FInventoryItemManifest Manifest = LootGenerator::Generate(*Template, {LootPowerParamsTest::DoubleLevel, Seed});
		const FPowerGrid_ItemFragment* PowerGrid = PowerGridOf(Manifest);
		if (!TestNotNull(TEXT("Power grid fragment"), PowerGrid))
		{
			return false;
		}
		const FPowerSourceParams& Source = PowerGrid->Source;
		TestEqual(TEXT("Scaling power doubles at level 21"), Source.Power, 6);
		TestEqual(TEXT("Ammo type is fixed"), Source.AmmoType, ShootingTags::Ammo_Bullet.GetTag());
		TestTrue(FString::Printf(TEXT("Capacity %d in [10, 20]"), Source.Capacity), Source.Capacity >= 10 && Source.Capacity <= 20);
		TestEqual(TEXT("Non-scaling cost per use"), Source.CostPerUse, 2);
		TestEqual(TEXT("Recharge delay"), Source.RechargeDelay, 0.5f);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootPowerParamsReceiverTest, "TypicalShooter.Loot.Generator.PowerParams.Receiver", LootTestFixture::Flags)
bool FLootPowerParamsReceiverTest::RunTest(const FString& Parameters)
{
	using namespace LootTestFixture;
	ULootItemTemplate* Template = MakeTemplate(EPowerItemType::Receiver, {}, false);
	Template->Receiver.Ability = UGameplayAbility::StaticClass();
	Template->Receiver.RequiredPower = FLootIntRange(2, 2, true);
	FLootAttackStatRange Damage;
	Damage.Attribute = EAttackAttribute::DamagePhysical;
	Damage.Value = FLootFloatRange(10.f, 10.f, true);
	FLootAttackStatRange Speed;
	Speed.Attribute = EAttackAttribute::ProjectileSpeed;
	Speed.Value = FLootFloatRange(1000.f, 2000.f);
	Template->Receiver.BaseStats = {Damage, Speed};

	const FInventoryItemManifest Manifest = LootGenerator::Generate(*Template, {LootPowerParamsTest::DoubleLevel, 9});
	const FPowerGrid_ItemFragment* PowerGrid = PowerGridOf(Manifest);
	if (!TestNotNull(TEXT("Power grid fragment"), PowerGrid))
	{
		return false;
	}
	const FPowerReceiverParams& Receiver = PowerGrid->Receiver;
	TestEqual(TEXT("Ability is fixed"), Receiver.Ability.Get(), UGameplayAbility::StaticClass());
	TestEqual(TEXT("Scaling required power doubles"), Receiver.RequiredPower, 4);
	if (TestEqual(TEXT("Every base stat is rolled"), Receiver.BaseStats.Num(), 2))
	{
		TestEqual(TEXT("First stat attribute"), Receiver.BaseStats[0].Attribute, EAttackAttribute::DamagePhysical);
		TestEqual(TEXT("Scaling stat doubles"), Receiver.BaseStats[0].Value, 20.f);
		TestEqual(TEXT("Second stat attribute"), Receiver.BaseStats[1].Attribute, EAttackAttribute::ProjectileSpeed);
		TestTrue(TEXT("Non-scaling stat stays in range"), Receiver.BaseStats[1].Value >= 1000.f && Receiver.BaseStats[1].Value <= 2000.f);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootPowerParamsConnectorTest, "TypicalShooter.Loot.Generator.PowerParams.Connector", LootTestFixture::Flags)
bool FLootPowerParamsConnectorTest::RunTest(const FString& Parameters)
{
	using namespace LootTestFixture;
	ULootItemTemplate* Template = MakeTemplate(EPowerItemType::Connector, {}, false);
	Template->Source.Power = FLootIntRange(9, 9);
	Template->Receiver.RequiredPower = FLootIntRange(9, 9);

	const FInventoryItemManifest Manifest = LootGenerator::Generate(*Template, {1, 1});
	const FPowerGrid_ItemFragment* PowerGrid = PowerGridOf(Manifest);
	if (TestNotNull(TEXT("Power grid fragment"), PowerGrid))
	{
		TestEqual(TEXT("Connector ignores source ranges"), PowerGrid->Source.Power, FPowerSourceParams().Power);
		TestEqual(TEXT("Connector ignores receiver ranges"), PowerGrid->Receiver.RequiredPower, FPowerReceiverParams().RequiredPower);
	}
	return true;
}

#endif
