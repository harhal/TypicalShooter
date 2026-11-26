// Tests for items containers filled from a loot table (LootGeneration R9).

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "InventoryComponent.h"
#include "InventoryItem.h"
#include "ItemsContainer.h"
#include "LootTestFixture.h"
#include "Loot/LootTable.h"
#include "ShootingTestFixture.h"

namespace LootContainerTest
{
	using namespace LootTestFixture;

	ULootTable* ConnectorTable(const int32 Count)
	{
		ULootTable* Table = NewObject<ULootTable>();
		Table->ItemCount = FLootIntRange(Count, Count);
		FLootTemplateEntry& Entry = Table->Templates.AddDefaulted_GetRef();
		Entry.Template = MakeTemplate(EPowerItemType::Connector, {}, true);
		return Table;
	}

	AItemsContainer* SpawnContainer(UWorld& World, const ULootTable* Table, const int32 Level, const int32 Seed, const int32 HandMade = 0)
	{
		AItemsContainer* Container = World.SpawnActorDeferred<AItemsContainer>(AItemsContainer::StaticClass(), FTransform::Identity);
		Container->LootTable = Table;
		Container->LootLevel = Level;
		Container->LootSeed = Seed;
		for (int32 Index = 0; Index < HandMade; ++Index)
		{
			Container->ItemsManifest.Add(FInstancedStruct::Make(FInventoryItemManifest()));
		}
		Container->FinishSpawning(FTransform::Identity);
		return Container;
	}

	/** Seeds of the generated items, in placement order; hand-made items give -1. */
	TArray<int32> LootSeeds(const AItemsContainer& Container)
	{
		TArray<int32> Seeds;
		for (const TPair<UInventoryItem*, FHhAxialCoord>& Placed : Container.GetPlacedItems())
		{
			const FInventoryItemManifest* Manifest = Placed.Key ? Placed.Key->GetItemManifest() : nullptr;
			const FLootOrigin_ItemFragment* Origin = Manifest ? OriginOf(*Manifest) : nullptr;
			Seeds.Add(Origin ? Origin->Seed : -1);
		}
		return Seeds;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootContainerFillTest, "TypicalShooter.Loot.Container.Fill", LootTestFixture::Flags)
bool FLootContainerFillTest::RunTest(const FString& Parameters)
{
	using namespace LootContainerTest;
	const ShootingTest::FWorldFixture F(true);
	const ULootTable* Table = ConnectorTable(3);

	const AItemsContainer* Container = SpawnContainer(*F.World, Table, 7, 42, 2);
	TestEqual(TEXT("Hand-made items plus the table's items"), Container->InventoryComponent->GetNumItems(), 5);
	const TArray<int32> Seeds = LootSeeds(*Container);
	TestEqual(TEXT("Hand-made items stay"), Seeds.FilterByPredicate([](const int32 Seed) { return Seed == -1; }).Num(), 2);
	for (const TPair<UInventoryItem*, FHhAxialCoord>& Placed : Container->GetPlacedItems())
	{
		const FLootOrigin_ItemFragment* Origin = OriginOf(*Placed.Key->GetItemManifest());
		if (Origin)
		{
			TestEqual(TEXT("Loot uses the container level"), Origin->Level, 7);
		}
	}

	const AItemsContainer* Twin = SpawnContainer(*F.World, Table, 7, 42, 2);
	TestEqual(TEXT("Same table, level and seed -> same items"), LootSeeds(*Twin), Seeds);

	const AItemsContainer* Plain = SpawnContainer(*F.World, nullptr, 7, 42, 2);
	TestEqual(TEXT("Without a table only hand-made items"), Plain->InventoryComponent->GetNumItems(), 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootContainerOverflowTest, "TypicalShooter.Loot.Container.Overflow", LootTestFixture::Flags)
bool FLootContainerOverflowTest::RunTest(const FString& Parameters)
{
	using namespace LootContainerTest;
	const ShootingTest::FWorldFixture F(true);
	AddExpectedMessage(TEXT("did not fit"), EAutomationExpectedMessageFlags::Contains, 1);

	const AItemsContainer* Container = SpawnContainer(*F.World, ConnectorTable(25), 1, 5);
	TestEqual(TEXT("Only as many items as there are cells"), Container->InventoryComponent->GetNumItems(), 19);

	const AItemsContainer* RandomSeed = SpawnContainer(*F.World, ConnectorTable(2), 1, 0);
	TestEqual(TEXT("Seed 0 still fills the container"), RandomSeed->InventoryComponent->GetNumItems(), 2);
	return true;
}

#endif
