// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoreFramework/Interface/PTWInteractInterface.h"
#include "PTWInteractableActor.generated.h"

UCLASS()
class PTW_API APTWInteractableActor : public AActor, public IPTWInteractInterface
{
	GENERATED_BODY()

public:
	APTWInteractableActor();

protected:
	virtual void OnInteract_Implementation(APawn* InstigatorPawn) override;
	virtual FText GetInteractionKeyword_Implementation() override;
	virtual bool IsInteractable_Implementation() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText InteractionText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bIsInteractable = true;
};
