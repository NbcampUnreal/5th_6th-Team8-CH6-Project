// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DetectorActor.h"
#include "ChargeDetectActor.generated.h"

class UGameplayAbility;

UCLASS()
class PTW_API AChargeDetectActor : public ADetectorActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AChargeDetectActor();

protected:
	virtual void OnDetectOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	
protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayAbility> ChargeAbilityClass;
};
