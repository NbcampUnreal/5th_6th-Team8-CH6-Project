// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "GC_Blind.generated.h"

UCLASS()
class PTW_API AGC_Blind : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGC_Blind();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	
	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
