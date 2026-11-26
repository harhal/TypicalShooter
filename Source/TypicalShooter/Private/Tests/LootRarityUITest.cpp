// Tests for the rarity shown by the inventory UI: grid frame color, info panel name color and rarity line
// (LootGeneration R10). No window: presenters + Slate views.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "LootTestFixture.h"
#include "Loot/LootGenerator.h"
#include "Tests/InventoryTestFixture.h"
#include "UI/InventoryItem/InventoryItem_Presenter.h"
#include "UI/ItemsContainer/ItemsContainer_Presenter.h"
#include "UI/Slate/InventoryGrid.h"
#include "UI/Slate/InventoryGridItem.h"
#include "UI/Slate/InventoryItemInfo.h"
#include "UObject/StrongObjectPtr.h"

namespace LootRarityUITest
{
	const FLinearColor EpicColor(0.6f, 0.2f, 0.9f, 1.f);
	const FHhAxialCoord LootCell(0, 0);
	const FHhAxialCoord PlainCell(1, 0);

	/** Inventory with a generated "Epic" item at LootCell and a hand-made one at PlainCell. */
	struct FRarityFixture : InventoryCoreTest::FFixture
	{
		TStrongObjectPtr<ULootRarity> Epic;
		UInventoryItem* LootItem = nullptr;
		UInventoryItem* PlainItem = nullptr;

		FRarityFixture()
		{
			Epic.Reset(LootTestFixture::MakeRarity({}));
			Epic->DisplayName = FText::FromString(TEXT("Epic"));
			Epic->Color = EpicColor;
			const ULootItemTemplate* Template = LootTestFixture::MakeTemplate(EPowerItemType::Connector, {}, false);
			LootItem = Inventory->AddItem(LootGenerator::Generate(*Template, {1, 1, Epic.Get()}), LootCell);
			PlainItem = Inventory->AddItem(InventoryCoreTest::NamedManifest(TEXT("Medkit")), PlainCell);
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootRarityModelTest, "TypicalShooter.Loot.UI.RarityModel", LootTestFixture::Flags)
bool FLootRarityModelTest::RunTest(const FString& Parameters)
{
	using namespace LootRarityUITest;
	const FRarityFixture F;
	const TOptional<FItemRarityDisplay> Rarity = F.LootItem->GetItemRarity();
	if (TestTrue(TEXT("Generated item has a rarity"), Rarity.IsSet()))
	{
		TestEqual(TEXT("Rarity name"), Rarity->Name.ToString(), FString(TEXT("Epic")));
		TestEqual(TEXT("Rarity color"), Rarity->Color, EpicColor);
	}
	TestFalse(TEXT("Hand-made item has no rarity"), F.PlainItem->GetItemRarity().IsSet());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootRarityGridTest, "TypicalShooter.Loot.UI.RarityGridFrame", LootTestFixture::Flags)
bool FLootRarityGridTest::RunTest(const FString& Parameters)
{
	using namespace LootRarityUITest;
	const FRarityFixture F;
	const TStrongObjectPtr<UItemsContainer_Presenter> Presenter(UItemsContainer_Presenter::CreatePresenter(F.Inventory));
	const TSharedRef<SInventoryGrid> Grid = SNew(SInventoryGrid);
	Presenter->SetView(Grid);

	const TSharedPtr<const SInventoryGridItem> LootView = Grid->GetItemViewAt(LootCell);
	const TSharedPtr<const SInventoryGridItem> PlainView = Grid->GetItemViewAt(PlainCell);
	if (TestTrue(TEXT("Both items have views"), LootView.IsValid() && PlainView.IsValid()))
	{
		TestEqual(TEXT("Generated item is framed in its rarity color"), LootView->GetFrameColor().Get(FLinearColor::Black), EpicColor);
		TestFalse(TEXT("Hand-made item has no frame"), PlainView->GetFrameColor().IsSet());
	}
	Presenter->SetView(nullptr);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLootRarityInfoTest, "TypicalShooter.Loot.UI.RarityInfoPanel", LootTestFixture::Flags)
bool FLootRarityInfoTest::RunTest(const FString& Parameters)
{
	using namespace LootRarityUITest;
	const FRarityFixture F;
	const TSharedRef<SInventoryItemInfo> Info = SNew(SInventoryItemInfo);
	const TStrongObjectPtr<UInventoryItem_Presenter> Presenter(NewObject<UInventoryItem_Presenter>());
	Presenter->SetView(Info);
	const FLinearColor PlainNameColor = Info->GetNameColor();

	Presenter->SetModel(F.LootItem);
	TestEqual(TEXT("Name is drawn in the rarity color"), Info->GetNameColor(), EpicColor);
	TestEqual(TEXT("Rarity line shows its name"), Info->GetDisplayedRarity().ToString(), FString(TEXT("Epic")));

	Presenter->SetModel(F.PlainItem);
	TestEqual(TEXT("Hand-made item: default name color"), Info->GetNameColor(), PlainNameColor);
	TestTrue(TEXT("Hand-made item: no rarity line"), Info->GetDisplayedRarity().IsEmpty());
	return true;
}

#endif
