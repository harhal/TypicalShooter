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

	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;
	
	virtual void PlayerTick(float DeltaTime) override;

	/** Interacts with the nearest interactive actor in range; closes the opened container if there is one. */
	UFUNCTION(BlueprintNativeEvent)
	void PrimaryInteract();

	/** Called (locally) after a container was opened by interaction. Show the container UI here. */
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	void OnItemsContainerOpened(AItemsContainer* Container);

	/** Called (locally) when the opened container is closed or went out of interaction range. Hide the UI here. */
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	void OnItemsContainerClosed();

public:
	UFUNCTION(BlueprintCallable, Category="Inventory")
	void CloseItemsContainer();

	UFUNCTION(BlueprintPure, Category="Inventory")
	AItemsContainer* GetOpenedItemsContainer() const { return OpenedContainer.Get(); }

private:
	/** Nearest actor implementing IPlayerInteractive among the pawn's interaction overlaps. */
	AActor* FindInteractionTarget() const;
	bool IsInInteractionRange(const AActor* Actor) const;

	TWeakObjectPtr<AItemsContainer> OpenedContainer;
	bool bContainerOpened = false;
};
