// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTWRedLightFinishBox.generated.h"

class UBoxComponent;

UCLASS()
class PTW_API APTWRedLightFinishBox : public AActor
{
	GENERATED_BODY()
	
public:
	APTWRedLightFinishBox();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Finish")
	TObjectPtr<UBoxComponent> FinishVolume;
};
