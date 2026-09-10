// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemsContainer.h"

#include "HhGridFunctions.h"
#include "Components/SphereComponent.h"
#include "InventoryComponent.h"
#include "Net/UnrealNetwork.h"


AItemsContainer::AItemsContainer(const FObjectInitializer& ObjectInitializer) : 
	Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	
	SphereCollision = ObjectInitializer.CreateDefaultSubobject<USphereComponent>(this, TEXT("SphereCollision"));
	SphereCollision->SetCollisionObjectType(ECC_GameTraceChannel2);
	SphereCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereCollision->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Overlap);
	
	InventoryComponent = ObjectInitializer.CreateDefaultSubobject<UInventoryComponent>(this, TEXT("InventoryComponent"));
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
		FHhAxialCoord ItemCoords = UHhGridFunctions::MakeHexAxialCoordFromSpiral(ItemIdx);
		InventoryComponent->AddItem(ItemManifest.Get<FInventoryItemManifest>(), ItemCoords);
		ItemIdx++;		
	}
}

void AItemsContainer::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, ItemsManifest);
}

void AItemsContainer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

