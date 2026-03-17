// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RaceTrack.generated.h"

class USplineComponent;

UCLASS()
class PTW_API ARaceTrack : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARaceTrack();
	
	FORCEINLINE USplineComponent* GetSplineComponent() const { return Spline;} 

protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<USplineComponent> Spline;
};
