// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TypicalShooterPlayerController.generated.h"

class AItemsContainer;
class UInputAction;
class UInputMappingContext;

UCLASS(abstract, config="Game")
class TYPICALSHOOTER_API ATypicalShooterPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	ATypicalShooterPlayerController();

protected:

	UPROPERTY(EditAnywhere, Category="Input")
	TArray<TObjectPtr<UInputMappingContext>> DefaultIMCs;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> PrimaryInteractAction;

	/** Opens / closes the player's inventory screen. */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> ToggleInventoryAction;

	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;
	
	virtual void PlayerTick(float DeltaTime) override;

	/**
	 * Interacts with the nearest interactive actor in range. For a container this opens the inventory
	 * together with the container; pressing again while a container is open closes the inventory.
	 */
	UFUNCTION(BlueprintNativeEvent)
	void PrimaryInteract();

	void ToggleInventory();

	/** Called (locally) when the inventory screen opens. Show the inventory UI here. */
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	void OnInventoryOpened();

	/** Called (locally) when the inventory screen closes (the container, if any, is closed first). */
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	void OnInventoryClosed();

	/** Called (locally) after a container was opened by interaction. Show the container UI here. */
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	void OnItemsContainerOpened(AItemsContainer* Container);

	/** Called (locally) when the opened container is closed or went out of interaction range. Hide the UI here. */
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	void OnItemsContainerClosed();

public:
	/** Shows the inventory screen: mouse cursor on, game + UI input. */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	void OpenInventory();

	/** Hides the inventory screen (and the opened container): back to game-only input. */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	void CloseInventory();

	UFUNCTION(BlueprintPure, Category="Inventory")
	bool IsInventoryOpened() const { return bInventoryOpened; }

	/** Closes only the container part of the inventory screen. */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	void CloseItemsContainer();

	UFUNCTION(BlueprintPure, Category="Inventory")
	AItemsContainer* GetOpenedItemsContainer() const { return OpenedContainer.Get(); }

private:
	/** Nearest actor implementing IPlayerInteractive among the pawn's interaction overlaps. */
	AActor* FindInteractionTarget() const;
	bool IsInInteractionRange(const AActor* Actor) const;

	/** Game-only input with hidden cursor, or game + UI input with visible cursor. */
	void ApplyInputMode(bool bUIVisible);

	TWeakObjectPtr<AItemsContainer> OpenedContainer;
	bool bContainerOpened = false;
	bool bInventoryOpened = false;
};
