// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BarrierActor.generated.h"

class UBoxComponent;

UCLASS()
class PTW_API ABarrierActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABarrierActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UBoxComponent> BoxComponent;
	
	FTimerHandle TimerHandle;
};
