// Shared fixture for inventory tests: a bare game world with actors owning UInventoryComponents (no PIE).

#pragma once

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "InventoryComponent.h"
#include "InventoryItem.h"
#include "InventoryItemManifest.h"
#include "ItemFragment.h"

namespace InventoryCoreTest
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** Temporary game world + actor with an inventory component. */
	struct FFixture
	{
		UWorld* World = nullptr;
		AActor* Actor = nullptr;
		UInventoryComponent* Inventory = nullptr;

		int32 NumAdded = 0;
		int32 NumRemoved = 0;
		int32 NumMoved = 0;
		const UInventoryItem* LastItem = nullptr;
		FHhAxialCoord LastLocation;
		FHhAxialCoord LastPrevLocation;

		FFixture()
		{
			World = UWorld::CreateWorld(EWorldType::Game, false);
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());

			Actor = World->SpawnActor<AActor>();
			Inventory = NewObject<UInventoryComponent>(Actor, TEXT("Inventory"));
			Inventory->RegisterComponent();

			Inventory->GetItemAddedDelegate().AddLambda([this](const UInventoryItem* Item, const FHhAxialCoord& Location)
			{
				++NumAdded;
				LastItem = Item;
				LastLocation = Location;
			});
			Inventory->GetItemRemovedDelegate().AddLambda([this](const UInventoryItem* Item)
			{
				++NumRemoved;
				LastItem = Item;
			});
			Inventory->GetItemMovedDelegate().AddLambda([this](const UInventoryItem* Item, const FHhAxialCoord& Location, const FHhAxialCoord& Prev)
			{
				++NumMoved;
				LastItem = Item;
				LastLocation = Location;
				LastPrevLocation = Prev;
			});
		}

		~FFixture()
		{
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}

		UInventoryItem* Add(const FHhAxialCoord& Location) const
		{
			return Inventory->AddItem(FInventoryItemManifest(), Location);
		}

		/** Another actor with its own inventory in the same world (e.g. a container next to the player). */
		UInventoryComponent* MakeInventory(const int32 GridRadius = 2) const
		{
			AActor* OtherActor = World->SpawnActor<AActor>();
			UInventoryComponent* Other = NewObject<UInventoryComponent>(OtherActor, TEXT("Inventory"));
			Other->SetGridRadius(GridRadius);
			Other->RegisterComponent();
			return Other;
		}

		/** Location of Item according to GetPlacedItems(), or unset if not placed. */
		TOptional<FHhAxialCoord> LocationOf(const UInventoryItem* Item) const
		{
			return LocationIn(Inventory, Item);
		}

		static TOptional<FHhAxialCoord> LocationIn(const UInventoryComponent* InInventory, const UInventoryItem* Item)
		{
			for (const TPair<UInventoryItem*, FHhAxialCoord>& Placed : InInventory->GetPlacedItems())
			{
				if (Placed.Key == Item)
				{
					return Placed.Value;
				}
			}
			return {};
		}

		/** Item placed at Location in InInventory, nullptr if the cell is empty. */
		static UInventoryItem* ItemAt(const UInventoryComponent* InInventory, const FHhAxialCoord& Location)
		{
			for (const TPair<UInventoryItem*, FHhAxialCoord>& Placed : InInventory->GetPlacedItems())
			{
				if (Placed.Value == Location)
				{
					return Placed.Key;
				}
			}
			return nullptr;
		}
	};

	/** Manifest with an Info fragment, so items can be told apart by name. */
	inline FInventoryItemManifest NamedManifest(const TCHAR* Name)
	{
		FInventoryItemManifest Manifest;
		FInfo_ItemFragment Info;
		Info.DisplayName = FText::FromString(Name);
		Manifest.Fragments.Add(TInstancedStruct<FItemFragment>::Make<FInfo_ItemFragment>(Info));
		return Manifest;
	}

	inline FString NameOf(const UInventoryItem* Item)
	{
		return Item ? Item->GetItemName().ToString() : FString(TEXT("<none>"));
	}

	const FHhAxialCoord CellA(0, 0);
	const FHhAxialCoord CellB(1, 0);
	const FHhAxialCoord CellC(0, 1);
	const FHhAxialCoord CellD(-1, 1);
}


#endif // WITH_DEV_AUTOMATION_TESTS
