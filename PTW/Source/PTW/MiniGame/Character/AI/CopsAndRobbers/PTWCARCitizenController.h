// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MiniGame/Character/AI/PTWCitizenController.h"
#include "PTWCARCitizenController.generated.h"

UCLASS()
class PTW_API APTWCARCitizenController : public APTWCitizenController
{
	GENERATED_BODY()

public:
	APTWCARCitizenController();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	void MoveToRandomLocation();
	
protected:
	UPROPERTY(EditDefaultsOnly)
	float MoveRadius;
	FTimerHandle AIMoveTimerHandle;
public:
};
