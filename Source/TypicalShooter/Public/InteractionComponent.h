// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

UENUM()
enum EInteractionType
{
	None,
	Open
};

UINTERFACE(Blueprintable, BlueprintType)
class UPlayerInteractive : public UInterface
{
	GENERATED_BODY()
};

class IPlayerInteractive
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	EInteractionType GetInteractionType() const;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Interact(EInteractionType InteractionType);
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TYPICALSHOOTER_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent();

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	void GetOverlapActors(TArray<AActor*>& OutOverlapActors);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static TScriptInterface<IPlayerInteractive> GetInteractionAPI(AActor* Actor);
	
private:
	UPROPERTY(EditAnywhere)
	TEnumAsByte<ECollisionChannel> ItemTraceChannel = ECC_GameTraceChannel2;
	
	UPROPERTY(EditAnywhere)
	float TraceDistance = 100.0f;
	
	UPROPERTY()
	TArray<AActor*> OverlapActors;
	
	FTraceHandle TraceHandle = FTraceHandle();
	
	FOverlapDelegate OnUpdateOverlapList = FOverlapDelegate::CreateUObject(this, &UInteractionComponent::UpdateOverlapList);		
	void UpdateOverlapList(const FTraceHandle& CompleteTraceHandle, FOverlapDatum& OverlapDatum);
};
