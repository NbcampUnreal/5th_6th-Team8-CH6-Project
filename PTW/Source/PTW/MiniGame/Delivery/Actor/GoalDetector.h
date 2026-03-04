// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DetectorBase.h"
#include "GoalDetector.generated.h"

UCLASS()
class PTW_API AGoalDetector : public ADetectorBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGoalDetector();
	
protected:
	UFUNCTION()
	virtual void OnDetectPlayer(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
};
