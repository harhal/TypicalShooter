// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemsContainer.h"

#include "HhGridFunctions.h"
#include "Components/SphereComponent.h"
#include "InventoryComponent.h"
#include "PowerGrid/PowerGridComponent.h"
#include "Net/UnrealNetwork.h"
#include "TypicalShooter.h"


AItemsContainer::AItemsContainer(const FObjectInitializer& ObjectInitializer) : 
	Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	
	SphereCollision = ObjectInitializer.CreateDefaultSubobject<USphereComponent>(this, TEXT("SphereCollision"));
	SphereCollision->SetCollisionObjectType(ECC_GameTraceChannel2);
	SphereCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereCollision->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Overlap);
	
	InventoryComponent = ObjectInitializer.CreateDefaultSubobject<UInventoryComponent>(this, TEXT("InventoryComponent"));
	PowerGrid = ObjectInitializer.CreateDefaultSubobject<UPowerGridComponent>(this, TEXT("PowerGrid"));
}

void AItemsContainer::BeginPlay()
{
	Super::BeginPlay();
	
	if (!HasAuthority())
	{
		return;
	}
	
	int32 ItemIdx = 0;
	for (const FInstancedStruct& ItemManifest : ItemsManifest)
	{
		// Entries left empty in the editor have no struct set.
		const FInventoryItemManifest* Manifest = ItemManifest.GetPtr<FInventoryItemManifest>();
		if (!Manifest)
		{
			continue;
		}

		const FHhAxialCoord ItemCoords = UHhGridFunctions::MakeHexAxialCoordFromSpiral(ItemIdx);
		if (InventoryComponent->AddItem(*Manifest, ItemCoords))
		{
			ItemIdx++;
		}
	}

	UE_LOG(LogTypicalShooter, Log, TEXT("%s: placed %d of %d manifest items"),
		*GetName(), InventoryComponent->GetNumItems(), ItemsManifest.Num());
}

void AItemsContainer::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, ItemsManifest);
}

TArray<TPair<UInventoryItem*, FHhAxialCoord>> AItemsContainer::GetPlacedItems() const
{
	return InventoryComponent->GetPlacedItems();
}

FItemAddedDelegate& AItemsContainer::GetItemAddedDelegate()
{
	return InventoryComponent->GetItemAddedDelegate();
}

FItemRemovedDelegate& AItemsContainer::GetItemRemovedDelegate()
{
	return InventoryComponent->GetItemRemovedDelegate();
}

FItemMovedDelegate& AItemsContainer::GetItemMovedDelegate()
{
	return InventoryComponent->GetItemMovedDelegate();
}

void AItemsContainer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


int32 AItemsContainer::GetGridRadius() const
{
	return InventoryComponent->GetGridRadius();
}

void AItemsContainer::RequestTransferItem(UInventoryItem* Item, const TScriptInterface<IItemsContainer_Model>& Target, const FHhAxialCoord& Location)
{
	InventoryComponent->RequestTransferItem(Item, Target, Location);
}

bool AItemsContainer::CanBeAccessedBy(const AActor* Accessor) const
{
	if (!Accessor)
	{
		return false;
	}
	const float Reach = SphereCollision->GetScaledSphereRadius() + AccessRangeTolerance;
	return FVector::DistSquared(Accessor->GetActorLocation(), GetActorLocation()) <= FMath::Square(Reach);
}
