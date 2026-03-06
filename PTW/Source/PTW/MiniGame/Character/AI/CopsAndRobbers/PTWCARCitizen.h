// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MiniGame/Character/AI/PTWBaseCitizen.h"
#include "PTWCARCitizen.generated.h"

UCLASS()
class PTW_API APTWCARCitizen : public APTWBaseCitizen
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APTWCARCitizen();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
