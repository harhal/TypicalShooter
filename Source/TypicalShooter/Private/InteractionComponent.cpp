// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionComponent.h"
#include "Engine/OverlapResult.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!TraceHandle.IsValid())
	{	
		FCollisionShape TraceSphere = FCollisionShape::MakeSphere(TraceDistance);
		TraceHandle = GetWorld()->AsyncOverlapByChannel(
			GetOwner()->GetActorLocation(), FQuat::Identity, ItemTraceChannel, TraceSphere, 
			FCollisionQueryParams::DefaultQueryParam, FCollisionResponseParams::DefaultResponseParam,
			&OnUpdateOverlapList);
	}
}

void UInteractionComponent::GetOverlapActors(TArray<AActor*>& OutOverlapActors)
{
	OutOverlapActors = OverlapActors;
}

TScriptInterface<IPlayerInteractive> UInteractionComponent::GetInteractionAPI(AActor* Actor)
{	
	if (Actor->Implements<UPlayerInteractive>())
	{
		return TScriptInterface<IPlayerInteractive>(Actor);
	}
	
	return nullptr;
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void UInteractionComponent::UpdateOverlapList(const FTraceHandle& CompleteTraceHandle, FOverlapDatum& OverlapDatum)
{
	TraceHandle = FTraceHandle();
	
	OverlapActors.Empty();
	OverlapActors.Reserve(OverlapDatum.OutOverlaps.Num());
	
	Algo::Transform(OverlapDatum.OutOverlaps, OverlapActors, [](const FOverlapResult& Overlap)
	{
	
	UE_LOG(LogActor, Log, TEXT("Overlap actor %s"), *Overlap.GetActor()->GetName());
		return Overlap.GetActor();
	});
}

