// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DetectorActor.generated.h"

class UGameplayEffect;
class UBoxComponent;

UCLASS(Abstract)
class PTW_API ADetectorActor : public AActor
{
	GENERATED_BODY()

public:
	ADetectorActor();

protected:
	UFUNCTION()
	virtual void OnDetectOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<USceneComponent> SceneComponent;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> BoxComponent;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> EffectToApply;
};
