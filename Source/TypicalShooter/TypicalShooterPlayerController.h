// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TypicalShooterPlayerController.generated.h"

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
	
	UFUNCTION(BlueprintNativeEvent)
	void PrimaryInteract();
};
