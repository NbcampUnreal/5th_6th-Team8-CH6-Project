// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DetectorActor.h"
#include "SavePointDetectActor.generated.h"

class USoundCue;
class APTWPlayerCharacter;

UCLASS()
class PTW_API ASavePointDetectActor : public ADetectorActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASavePointDetectActor();

protected:
	virtual void OnDetectOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	
	void SpeedUpPlaySound(APTWPlayerCharacter* TargetCharacter);
	
protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundCue> SpeedUpSound;
};
