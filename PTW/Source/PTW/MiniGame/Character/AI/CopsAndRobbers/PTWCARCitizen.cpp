// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWCARCitizen.h"


// Sets default values
APTWCARCitizen::APTWCARCitizen()
{
	PrimaryActorTick.bCanEverTick = false;
	
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void APTWCARCitizen::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APTWCARCitizen::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void APTWCARCitizen::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

