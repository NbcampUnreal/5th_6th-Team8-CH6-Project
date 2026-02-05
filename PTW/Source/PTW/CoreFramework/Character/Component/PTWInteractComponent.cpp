// Fill out your copyright notice in the Description page of Project Settings.


#include "CoreFramework/Character/Component/PTWInteractComponent.h"
#include "CoreFramework/Interface/PTWInteractInterface.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "Components/PrimitiveComponent.h"
#include "Components/MeshComponent.h"

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

	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC || !PC->PlayerCameraManager) return;

	FVector TraceStart = PC->PlayerCameraManager->GetCameraLocation();
	FVector CameraForward = PC->PlayerCameraManager->GetCameraRotation().Vector();
	FVector TraceEnd = TraceStart + (CameraForward * InteractionDistance);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerPawn);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, TraceChannel, QueryParams);
	AActor* HitActor = bHit ? HitResult.GetActor() : nullptr;

	if (HitActor && HitActor->Implements<UPTWInteractInterface>())
	{
		if (HitActor != CurrentInteractableActor)
		{
			if (CurrentInteractableActor)
			{
				ToggleHighlight(CurrentInteractableActor, false);
			}
			CurrentInteractableActor = HitActor;
			ToggleHighlight(CurrentInteractableActor, true);

			FText ActionText = IPTWInteractInterface::Execute_GetInteractionKeyword(HitActor);
			OnInteractableFound.Broadcast(ActionText);
		}
	}
	else
	{
		if (CurrentInteractableActor)
		{
			ToggleHighlight(CurrentInteractableActor, false);
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

void UPTWInteractComponent::ToggleHighlight(AActor* TargetActor, bool bEnable)
{
	if (!TargetActor) return;

	TArray<UMeshComponent*> Meshes;
	TargetActor->GetComponents(Meshes);

	for (UMeshComponent* Mesh : Meshes)
	{
		if (bEnable)
		{
			if (OutlineOverlayMaterial)
			{
				Mesh->SetOverlayMaterial(OutlineOverlayMaterial);
			}
		}
		else
		{
			Mesh->SetOverlayMaterial(nullptr);
		}
	}
}

AActor* UPTWInteractComponent::GetInteractTargetUnsafe()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return nullptr;

	AController* Controller = OwnerPawn->GetController();
	if (!Controller) return nullptr;

	FVector TraceStart;
	FRotator TraceRot;
	Controller->GetPlayerViewPoint(TraceStart, TraceRot);

	FVector TraceEnd = TraceStart + (TraceRot.Vector() * InteractionDistance);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerPawn);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, TraceChannel, QueryParams);

	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor && HitActor->Implements<UPTWInteractInterface>())
		{
			return HitActor;
		}
	}

	return nullptr;
}
