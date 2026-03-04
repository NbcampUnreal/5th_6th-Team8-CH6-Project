// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DetectorBase.h"
#include "StartDetector.generated.h"

UCLASS()
class PTW_API AStartDetector : public ADetectorBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AStartDetector();
	
protected:
	virtual void OnDetectPlayer(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
};
