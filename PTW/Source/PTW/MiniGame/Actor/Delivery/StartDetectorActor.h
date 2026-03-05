// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DetectorActor.h"
#include "StartDetectorActor.generated.h"

UCLASS()
class PTW_API AStartDetectorActor : public ADetectorActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AStartDetectorActor();

protected:
	virtual void OnDetectOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
};
