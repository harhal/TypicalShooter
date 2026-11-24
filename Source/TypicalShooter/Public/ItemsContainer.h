// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractionComponent.h"
#include "GameFramework/Actor.h"
#include "StructUtils/InstancedStruct.h"
#include "UI/ItemsContainer/ItemsContainer_Model.h"
#include "ItemsContainer.generated.h"

class UInventoryComponent;
class USphereComponent;

UCLASS()
class TYPICALSHOOTER_API AItemsContainer : public AActor, public IPlayerInteractive, public IItemsContainer_Model
{
	GENERATED_BODY()

public:
	AItemsContainer(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// IItemsContainer_Model, forwarded to InventoryComponent
	virtual TArray<TPair<UInventoryItem*, FHhAxialCoord>> GetPlacedItems() const override;
	virtual FItemAddedDelegate& GetItemAddedDelegate() override;
	virtual FItemRemovedDelegate& GetItemRemovedDelegate() override;
	virtual FItemMovedDelegate& GetItemMovedDelegate() override;
	virtual int32 GetGridRadius() const override;
	virtual void RequestTransferItem(UInventoryItem* Item, const TScriptInterface<IItemsContainer_Model>& Target, const FHhAxialCoord& Location) override;

	/** Server-side check for item transfers: Accessor stands close enough to use this container. */
	bool CanBeAccessedBy(const AActor* Accessor) const;

	/** Extra distance (beyond the interaction sphere) tolerated by CanBeAccessedBy, covers the player interaction reach and latency. */
	UPROPERTY(EditAnywhere, Category = "Inventory")
	float AccessRangeTolerance = 250.f;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	USphereComponent* SphereCollision;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	UInventoryComponent* InventoryComponent;
	
	UPROPERTY(EditAnywhere, meta = (BaseStruct = "/Script/TypicalShooter.InventoryItemManifest"), Replicated)
	TArray<FInstancedStruct> ItemsManifest;
};
