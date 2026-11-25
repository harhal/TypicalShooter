#include "PowerGrid/DefaultLoadout.h"

#include "PowerGrid/PowerGridFragment.h"
#include "PowerGrid/ReceiverSlots.h"
#include "Shooting/DefaultAttackSetup.h"

#define LOCTEXT_NAMESPACE "DefaultLoadout"

namespace DefaultLoadout
{
	namespace Private
	{
		FInventoryItemManifest MakeManifest(const FPowerGrid_ItemFragment& Power, const FText& Name, const FText& Description)
		{
			FInfo_ItemFragment Info;
			Info.DisplayName = Name;
			Info.Description = Description;
			FInventoryItemManifest Manifest;
			Manifest.Fragments.Add(TInstancedStruct<FItemFragment>::Make<FInfo_ItemFragment>(Info));
			Manifest.Fragments.Add(TInstancedStruct<FItemFragment>::Make<FPowerGrid_ItemFragment>(Power));
			return Manifest;
		}

		FPowerGrid_ItemFragment MakeReceiver(const TSubclassOf<UGameplayAbility> Ability, const TArray<FAttackStatBonus>& BaseStats)
		{
			FPowerGrid_ItemFragment Power;
			Power.ItemType = EPowerItemType::Receiver;
			Power.Receiver.Ability = Ability;
			Power.Receiver.BaseStats = BaseStats;
			return Power;
		}
	}

	FInventoryItemManifest MakeStarterSource()
	{
		FPowerGrid_ItemFragment Power;
		Power.ItemType = EPowerItemType::Source;
		Power.Source.Power = 1;
		return Private::MakeManifest(Power, LOCTEXT("SourceName", "Starter Core"),
			LOCTEXT("SourceDescription", "Power source: 1 power on every face, needs no ammo."));
	}

	FInventoryItemManifest MakeRifleReceiver()
	{
		using A = EAttackAttribute;
		return Private::MakeManifest(Private::MakeReceiver(UPrimaryAttackAbility::StaticClass(), {
				{ A::FireEndDistance, 8000.f }, { A::BurstInterval, 0.12f }, { A::ProjectileSpeed, 8000.f }, { A::DamagePhysical, 10.f } }),
			LOCTEXT("RifleName", "Rifle Receiver"),
			LOCTEXT("RifleDescription", "Receiver: automatic fire while held. Needs 1 power."));
	}

	FInventoryItemManifest MakeShotgunReceiver()
	{
		using A = EAttackAttribute;
		return Private::MakeManifest(Private::MakeReceiver(UAlternateAttackAbility::StaticClass(), {
				{ A::FireEndDistance, 3000.f }, { A::BurstInterval, 0.8f }, { A::ProjectileSpeed, 6000.f },
				{ A::GravityScale, 0.2f }, { A::DamagePhysical, 6.f } }),
			LOCTEXT("ShotgunName", "Shotgun Receiver"),
			LOCTEXT("ShotgunDescription", "Receiver: seven pellets per shot. Needs 1 power."));
	}

	FInventoryItemManifest MakeDoubleJumpReceiver()
	{
		using A = EAttackAttribute;
		return Private::MakeManifest(Private::MakeReceiver(UDoubleJumpAbility::StaticClass(), {
				{ A::BurstInterval, 0.5f }, { A::Recoil, 900.f } }),
			LOCTEXT("DoubleJumpName", "Double Jump Receiver"),
			LOCTEXT("DoubleJumpDescription", "Receiver: no projectile, only a push away from the aim. In the air slot, jumping in the air throws you up. Needs 1 power."));
	}

	TArray<FStartingInventoryItem> MakeStartingItems()
	{
		return {
			{ MakeStarterSource(), FHhAxialCoord(0, 0) },
			{ MakeRifleReceiver(), ReceiverSlots::GetLocation(EReceiverSlot::Primary) },
			{ MakeShotgunReceiver(), ReceiverSlots::GetLocation(EReceiverSlot::Alternate) },
			{ MakeDoubleJumpReceiver(), ReceiverSlots::GetLocation(EReceiverSlot::Air) } };
	}
}

#undef LOCTEXT_NAMESPACE
