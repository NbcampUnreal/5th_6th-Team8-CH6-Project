// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/PTWInteractableActor.h"

APTWInteractableActor::APTWInteractableActor()
{
	PrimaryActorTick.bCanEverTick = false;
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	InteractionText = FText::FromString(TEXT("상호작용"));
}

void APTWInteractableActor::OnInteract_Implementation(APawn* InstigatorPawn)
{
	UE_LOG(LogTemp, Log, TEXT("[%s] Interacted with [%s]"), *InstigatorPawn->GetName(), *GetName());
}

FText APTWInteractableActor::GetInteractionKeyword_Implementation()
{
	return InteractionText;
}

bool APTWInteractableActor::IsInteractable_Implementation()
{
	return bIsInteractable;
}

