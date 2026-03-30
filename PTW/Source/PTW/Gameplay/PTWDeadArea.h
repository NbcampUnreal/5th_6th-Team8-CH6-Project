// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTWDeadArea.generated.h"

class UGameplayEffect;
class UBoxComponent;

UCLASS()
class PTW_API APTWDeadArea : public AActor
{
	GENERATED_BODY()
	
public:	
	APTWDeadArea();

protected:
	virtual void BeginPlay() override;
	UFUNCTION()
	void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	
private:

	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> DeadEffectClass;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> BoxComponent; 
};
