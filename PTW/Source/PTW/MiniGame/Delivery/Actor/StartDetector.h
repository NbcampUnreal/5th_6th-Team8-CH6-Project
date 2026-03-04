// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StartDetector.generated.h"

class UBoxComponent;

UCLASS()
class PTW_API AStartDetector : public AActor
{
	GENERATED_BODY()

public:
	AStartDetector();
	
protected:
	UFUNCTION()
	virtual void OnDetectPlayer(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, 
		bool bFromSweep, 
		const FHitResult& SweepResult);
	
protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> DetectArea;
};
