// Fill out your copyright notice in the Description page of Project Settings.


#include "GC_Blind.h"


// Sets default values
AGC_Blind::AGC_Blind()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AGC_Blind::BeginPlay()
{
	Super::BeginPlay();
}

bool AGC_Blind::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	UE_LOG(LogTemp, Warning, TEXT("AGC_Blind OnActive_Implementation"));
	return Super::OnActive_Implementation(MyTarget, Parameters);
}

bool AGC_Blind::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	UE_LOG(LogTemp, Warning, TEXT("AGC_Blind OnRemove_Implementation"));
	return Super::OnRemove_Implementation(MyTarget, Parameters);
}

// Called every frame
void AGC_Blind::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

