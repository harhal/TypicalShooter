// What the inventory UI shows about the power grid (power grid R16): per-item faces / power / clip, links, slot marks,
// and the info panel lines of an item (type, power, effect steps with the reached ones flagged).

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "PowerGrid/PowerEffectsFragment.h"
#include "PowerGrid/PowerGridComponent.h"
#include "PowerGrid/ReceiverAbilitiesComponent.h"
#include "Shooting/ShootingTags.h"
#include "Tests/InventoryTestFixture.h"
#include "Tests/PowerGridTestEffect.h"
#include "UI/PowerGrid/PowerGridDisplay.h"
#include "UI/InventoryItem/InventoryItem_Presenter.h"
#include "UI/Slate/InventoryItemInfo.h"
#include "UObject/StrongObjectPtr.h"

namespace PowerGridDisplayTest
{
	FInventoryItemManifest Power(const EPowerItemType Type, const int32 Faces = HexFaces::AllFaces, const int32 SourcePower = 1)
	{
		FPowerGrid_ItemFragment Fragment;
		Fragment.ItemType = Type;
		Fragment.OpenFaces = Faces;
		Fragment.Source.Power = SourcePower;
		Fragment.Receiver.RequiredPower = 3;
		FInventoryItemManifest Manifest;
		Manifest.Fragments.Add(TInstancedStruct<FItemFragment>::Make<FPowerGrid_ItemFragment>(Fragment));
		return Manifest;
	}

	FInventoryItemManifest WithSteps(FInventoryItemManifest Manifest)
	{
		FPowerEffects_ItemFragment Effects;
		FPowerThresholdEffect& Bonus = Effects.Effects.AddDefaulted_GetRef();
		Bonus.Threshold = 1;
		Bonus.AttackBonuses.Add({ EAttackAttribute::DamagePhysical, 5.f });
		FPowerThresholdEffect& Character = Effects.Effects.AddDefaulted_GetRef();
		Character.Threshold = 3;
		Character.CharacterEffect = UPowerGridTestEffect::StaticClass();
		Manifest.Fragments.Add(TInstancedStruct<FItemFragment>::Make<FPowerEffects_ItemFragment>(Effects));
		return Manifest;
	}

	FInventoryItemManifest AmmoSource()
	{
		FInventoryItemManifest Manifest = Power(EPowerItemType::Source);
		FPowerSourceParams& Source = Manifest.Fragments[0].GetMutable<FPowerGrid_ItemFragment>().Source;
		Source.AmmoType = ShootingTags::Ammo_Bullet;
		Source.Capacity = 4;
		return Manifest;
	}

	bool HasLink(const FPowerGridDisplay& Display, const FHhAxialCoord& From, const FHhAxialCoord& To, const bool bPowered)
	{
		return Display.Links.ContainsByPredicate([&](const FPowerLinkDisplay& Link)
		{
			return Link.From == From && Link.To == To && Link.bPowered == bPowered;
		});
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerGridDisplayBuildTest, "TypicalShooter.Inventory.UI.PowerGridDisplay", InventoryCoreTest::Flags)
bool FPowerGridDisplayBuildTest::RunTest(const FString& Parameters)
{
	using namespace PowerGridDisplayTest;
	InventoryCoreTest::FFixture F;
	UPowerGridComponent* Grid = NewObject<UPowerGridComponent>(F.Actor);
	Grid->RegisterComponent();

	F.Inventory->AddItem(Power(EPowerItemType::Source, HexFaces::AllFaces, 2), FHhAxialCoord(0, 0));
	UInventoryItem* Connector = F.Inventory->AddItem(Power(EPowerItemType::Connector, HexFaces::Bit(EHexFace::Left)), FHhAxialCoord(1, 0));
	F.Inventory->AddItem(Power(EPowerItemType::Receiver), FHhAxialCoord(0, -1));
	F.Inventory->AddItem(AmmoSource(), FHhAxialCoord(-2, 0));
	F.Inventory->AddItem(FInventoryItemManifest(), FHhAxialCoord(-1, 1));

	FPowerGridDisplay Display = PowerGridDisplay::Build(*Grid);
	TestEqual(TEXT("Only power items are shown"), Display.Cells.Num(), 4);
	const FPowerCellDisplay* Source = Display.FindCell(FHhAxialCoord(0, 0));
	const FPowerCellDisplay* Link = Display.FindCell(FHhAxialCoord(1, 0));
	const FPowerCellDisplay* Receiver = Display.FindCell(FHhAxialCoord(0, -1));
	const FPowerCellDisplay* Ammo = Display.FindCell(FHhAxialCoord(-2, 0));
	if (!TestTrue(TEXT("Every power item has a cell"), Source && Link && Receiver && Ammo))
	{
		return false;
	}
	TestTrue(TEXT("Source: type, power, all faces"), Source->Type == EPowerItemType::Source && Source->Power == 2 && Source->OpenFaces == HexFaces::AllFaces);
	TestTrue(TEXT("Source needs no ammo: no clip"), !Source->Clip.IsSet());
	TestEqual(TEXT("Connector faces as placed"), Link->OpenFaces, HexFaces::Bit(EHexFace::Left));
	TestTrue(TEXT("Connector fed"), Link->bPowered && Link->Power == 2);
	TestTrue(TEXT("Receiver: power and required power"), Receiver->Power == 2 && Receiver->RequiredPower == 3);
	TestTrue(TEXT("Ammo source: empty clip of 4, off"), Ammo->Clip == FIntPoint(0, 4) && !Ammo->bPowered);
	TestTrue(TEXT("Powered link source -> connector"), HasLink(Display, FHhAxialCoord(0, 0), FHhAxialCoord(1, 0), true));
	TestTrue(TEXT("Powered link source -> receiver"), HasLink(Display, FHhAxialCoord(0, 0), FHhAxialCoord(0, -1), true));
	TestFalse(TEXT("No slot marks without receiver abilities"), Display.bShowSlots);

	F.Inventory->RotateItem(Connector, 3);
	Display = PowerGridDisplay::Build(*Grid);
	TestEqual(TEXT("Faces follow the rotation"), Display.FindCell(FHhAxialCoord(1, 0))->OpenFaces, HexFaces::Bit(EHexFace::Right));
	TestFalse(TEXT("The closed side breaks the link"), HasLink(Display, FHhAxialCoord(0, 0), FHhAxialCoord(1, 0), true));

	NewObject<UReceiverAbilitiesComponent>(F.Actor)->RegisterComponent();
	TestTrue(TEXT("Slots are marked where receivers grant abilities"), PowerGridDisplay::Build(*Grid).bShowSlots);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerItemInfoTest, "TypicalShooter.Inventory.UI.PowerItemInfo", InventoryCoreTest::Flags)
bool FPowerItemInfoTest::RunTest(const FString& Parameters)
{
	using namespace PowerGridDisplayTest;
	InventoryCoreTest::FFixture F;
	UPowerGridComponent* Grid = NewObject<UPowerGridComponent>(F.Actor);
	Grid->RegisterComponent();
	F.Inventory->AddItem(Power(EPowerItemType::Source, HexFaces::AllFaces, 2), FHhAxialCoord(0, 0));
	UInventoryItem* Booster = F.Inventory->AddItem(WithSteps(Power(EPowerItemType::Connector)), FHhAxialCoord(1, 0));
	UInventoryItem* Receiver = F.Inventory->AddItem(Power(EPowerItemType::Receiver), FHhAxialCoord(0, -1));
	UInventoryItem* Ammo = F.Inventory->AddItem(AmmoSource(), FHhAxialCoord(-2, 0));
	UInventoryItem* Loose = F.Inventory->AddItem(WithSteps(FInventoryItemManifest()), FHhAxialCoord(-1, 1));
	UInventoryItem* Plain = F.Inventory->AddItem(FInventoryItemManifest(), FHhAxialCoord(1, 1));

	TestTrue(TEXT("The item's own grid is found"), PowerGridDisplay::FindGridOf(*Booster) == Grid);
	const FItemPowerInfo BoosterInfo = PowerGridDisplay::DescribeItem(*Booster, Grid);
	TestEqual(TEXT("Type line"), BoosterInfo.TypeText.ToString(), FString(TEXT("Connector")));
	TestTrue(TEXT("Power line shows the fed power"), BoosterInfo.PowerText.ToString().Contains(TEXT("2")));
	if (TestEqual(TEXT("One line per effect step"), BoosterInfo.Steps.Num(), 2))
	{
		TestTrue(TEXT("Step 1 reached at power 2"), BoosterInfo.Steps[0].bReached);
		TestTrue(TEXT("Step 1 names its bonus"), BoosterInfo.Steps[0].Text.ToString().Contains(TEXT("+5")));
		TestFalse(TEXT("Step 3 not reached"), BoosterInfo.Steps[1].bReached);
		TestTrue(TEXT("Step 3 names its threshold"), BoosterInfo.Steps[1].Text.ToString().Contains(TEXT("3")));
	}

	const FString ReceiverPower = PowerGridDisplay::DescribeItem(*Receiver, Grid).PowerText.ToString();
	TestTrue(FString::Printf(TEXT("Receiver power line has fed / required power (%s)"), *ReceiverPower),
		ReceiverPower.Contains(TEXT("2")) && ReceiverPower.Contains(TEXT("3")));
	const FString AmmoPower = PowerGridDisplay::DescribeItem(*Ammo, Grid).PowerText.ToString();
	TestTrue(FString::Printf(TEXT("Ammo source power line has its clip (%s)"), *AmmoPower), AmmoPower.Contains(TEXT("0/4")));

	const FItemPowerInfo LooseInfo = PowerGridDisplay::DescribeItem(*Loose, Grid);
	TestTrue(TEXT("An item outside the grid has no type"), LooseInfo.TypeText.IsEmpty());
	TestTrue(TEXT("... but its steps are listed, none reached at power 0"), LooseInfo.Steps.Num() == 2 && !LooseInfo.Steps[0].bReached);
	TestTrue(TEXT("A plain item has no power info"), PowerGridDisplay::DescribeItem(*Plain, Grid).IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerItemInfoPanelTest, "TypicalShooter.Inventory.UI.PowerItemInfoPanel", InventoryCoreTest::Flags)
bool FPowerItemInfoPanelTest::RunTest(const FString& Parameters)
{
	using namespace PowerGridDisplayTest;
	InventoryCoreTest::FFixture F;
	NewObject<UPowerGridComponent>(F.Actor)->RegisterComponent();
	UInventoryItem* Booster = F.Inventory->AddItem(WithSteps(Power(EPowerItemType::Connector)), FHhAxialCoord(1, 0));
	UInventoryItem* Plain = F.Inventory->AddItem(FInventoryItemManifest(), FHhAxialCoord(1, 1));

	const TSharedRef<SInventoryItemInfo> Info = SNew(SInventoryItemInfo);
	const TStrongObjectPtr<UInventoryItem_Presenter> Presenter(NewObject<UInventoryItem_Presenter>());
	Presenter->SetView(Info);
	Presenter->SetModel(Booster);
	TestEqual(TEXT("Panel shows the power type"), Info->GetPowerInfo().TypeText.ToString(), FString(TEXT("Connector")));
	TestEqual(TEXT("Panel lists the effect steps"), Info->GetNumPowerLines(), 4); // type, power, two steps
	TestFalse(TEXT("Unpowered: step 1 not reached"), Info->GetPowerInfo().Steps.Num() > 0 && Info->GetPowerInfo().Steps[0].bReached);

	F.Inventory->AddItem(Power(EPowerItemType::Source, HexFaces::AllFaces, 3), FHhAxialCoord(0, 0));
	TestTrue(TEXT("A power grid change refreshes the shown item: both steps reached"),
		Info->GetPowerInfo().Steps.Num() == 2 && Info->GetPowerInfo().Steps[0].bReached && Info->GetPowerInfo().Steps[1].bReached);
	TestTrue(TEXT("Reached steps are highlighted"), Info->IsPowerStepHighlighted(0));

	Presenter->SetModel(Plain);
	TestEqual(TEXT("A plain item shows no power lines"), Info->GetNumPowerLines(), 0);
	Presenter->SetView(nullptr);
	return true;
}

#endif
