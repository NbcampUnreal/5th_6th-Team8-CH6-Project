// Fill out your copyright notice in the Description page of Project Settings.


#include "CoreFramework/Character/Component/PTWInteractComponent.h"
#include "CoreFramework/Interface/PTWInteractInterface.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"

UPTWInteractComponent::UPTWInteractComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	CurrentInteractableActor = nullptr;
}

void UPTWInteractComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn && OwnerPawn->IsLocallyControlled())
	{
		TraceInteractable();
	}
}

void UPTWInteractComponent::TraceInteractable()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;

	AController* Controller = OwnerPawn->GetController();
	if (!Controller) return;

	FVector EyeLocation;
	FRotator EyeRotation;
	Controller->GetPlayerViewPoint(EyeLocation, EyeRotation);

	FVector TraceStart = EyeLocation;
	FVector TraceEnd = TraceStart + (EyeRotation.Vector() * InteractionDistance);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerPawn);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, TraceChannel, QueryParams);
	AActor* HitActor = bHit ? HitResult.GetActor() : nullptr;

	if (HitActor && HitActor->Implements<UPTWInteractInterface>())
	{
		if (HitActor != CurrentInteractableActor)
		{
			CurrentInteractableActor = HitActor;
			FText ActionText = IPTWInteractInterface::Execute_GetInteractionKeyword(HitActor);
			OnInteractableFound.Broadcast(ActionText);
		}
	}
	else
	{
		if (CurrentInteractableActor)
		{
			CurrentInteractableActor = nullptr;
			OnInteractableLost.Broadcast();
		}
	}
}

void UPTWInteractComponent::PerformInteraction()
{
	if (CurrentInteractableActor && CurrentInteractableActor->Implements<UPTWInteractInterface>())
	{
		if (IPTWInteractInterface::Execute_IsInteractable(CurrentInteractableActor))
		{
			APawn* Instigator = Cast<APawn>(GetOwner());
			IPTWInteractInterface::Execute_OnInteract(CurrentInteractableActor, Instigator);
		}
	}
}
