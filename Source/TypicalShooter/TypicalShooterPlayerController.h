// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HhAxialCoord.h"
#include "TypicalShooterPlayerController.generated.h"

class AItemsContainer;
class UInventoryComponent;
class UInventoryItem;

class UInputAction;
class UInputMappingContext;
class UInventoryUISubsystem;
struct FInputActionValue;

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

	/**
	 * Asks the server to put Item (placed in Source) at Location of Target (may equal Source; an occupied cell swaps).
	 * Both inventories must be accessible to this player (see CanAccessInventory).
	 */
	void RequestTransferItem(UInventoryComponent* Source, UInventoryItem* Item, UInventoryComponent* Target, const FHhAxialCoord& Location);

	/** Asks the server to rotate Item (placed in Inventory) by Steps clockwise 60-degree steps; Inventory must be accessible. */
	void RequestRotateItem(UInventoryComponent* Inventory, UInventoryItem* Item, int32 Steps = 1);

	/** Rotates the focused item (cursor / selection, else the hovered one) a step clockwise (power grid R3). */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	void RotateFocusedItem();

	/** The controlled pawn's own inventory, or a container the pawn stands next to. */
	bool CanAccessInventory(const UInventoryComponent* Inventory) const;

	/** Moves the item selected in the inventory UI from the opened container to the first free cell of the pawn's inventory. */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	void TakeSelectedItem();

	/** Like TakeSelectedItem, but for the item under the active gamepad / keyboard cursor if there is one. */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	void TakeFocusedItem();

	/** Moves every item of the opened container into the pawn's inventory (as many as fit). */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	void TakeAllFromOpenedContainer();

protected:
	UFUNCTION(Server, Reliable)
	void ServerTransferItem(UInventoryComponent* Source, UInventoryItem* Item, UInventoryComponent* Target, FHhAxialCoord Location);

	UFUNCTION(Server, Reliable)
	void ServerRotateItem(UInventoryComponent* Inventory, UInventoryItem* Item, int32 Steps);

	/** Items == empty takes everything Source holds. */
	UFUNCTION(Server, Reliable)
	void ServerTakeItems(UInventoryComponent* Source, const TArray<UInventoryItem*>& Items);

	/** Priority of InventoryIMC over the gameplay contexts (its actions consume their keys: A does not jump, the stick does not walk). */
	UPROPERTY(EditDefaultsOnly, Category="Input")
	int32 InventoryIMCPriority = 1;

private:
	UInventoryComponent* GetPawnInventory() const;
	UInventoryUISubsystem* GetInventoryUI() const;

	/** Asks the server to move Item from the opened container to the pawn's inventory. */
	void TakeItem(UInventoryItem* Item);

	/** Tells the UI cursor which grids are on screen: the pawn's inventory, then the opened container. */
	void UpdateNavigableContainers();

	// Gamepad / keyboard inventory controls. Built in code (no assets yet): see CreateInventoryInput for the keys.
	void CreateInventoryInput();
	void HandleInventoryNavigate(const FInputActionValue& Value);
	void HandleInventoryNavigateReleased();
	void HandleInventoryConfirm();
	void HandleInventoryCancel();
	void HandleInventorySwitchGrid(const FInputActionValue& Value);

	/** Always active: gamepad bindings for opening the inventory / interacting (IMC_Default has keyboard ones). */
	UPROPERTY(Transient)
	TObjectPtr<UInputMappingContext> GamepadIMC;

	/** Active while the inventory is open: cursor navigation, confirm / cancel, grid switching, take / take all, rotate. */
	UPROPERTY(Transient)
	TObjectPtr<UInputMappingContext> InventoryIMC;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> InventoryNavigateAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> InventoryConfirmAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> InventoryCancelAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> InventorySwitchGridAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> InventoryTakeAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> InventoryTakeAllAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> InventoryRotateAction;

	/** Held stick / D-pad direction: one step on press, then auto-repeat. */
	bool bNavigationHeld = false;
	int32 NavigationSector = 0;
	double NextNavigationRepeatTime = 0.0;

	/** Nearest actor implementing IPlayerInteractive among the pawn's interaction overlaps. */
	AActor* FindInteractionTarget() const;
	bool IsInInteractionRange(const AActor* Actor) const;

	/** Game-only input with hidden cursor, or game + UI input with visible cursor. */
	void ApplyInputMode(bool bUIVisible);

	TWeakObjectPtr<AItemsContainer> OpenedContainer;
	bool bContainerOpened = false;
	bool bInventoryOpened = false;
};
