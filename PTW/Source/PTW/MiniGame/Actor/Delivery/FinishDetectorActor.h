// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DetectorActor.h"
#include "FinishDetectorActor.generated.h"

UCLASS()
class PTW_API AFinishDetectorActor : public ADetectorActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFinishDetectorActor();

protected:
	virtual void OnDetectOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
};
