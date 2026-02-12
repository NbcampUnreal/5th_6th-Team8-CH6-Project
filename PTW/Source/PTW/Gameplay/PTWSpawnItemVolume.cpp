// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/PTWSpawnItemVolume.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "System/PTWItemSpawnManager.h"

APTWSpawnItemVolume::APTWSpawnItemVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	SpawnBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnBox"));
	RootComponent = SpawnBox;
	SpawnBox->SetCollisionProfileName(TEXT("NoCollision"));
}

void APTWSpawnItemVolume::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (UWorld* World = GetWorld())
		{
			if (UPTWItemSpawnManager* Manager = World->GetSubsystem<UPTWItemSpawnManager>())
			{
				Manager->RegisterSpawnVolume(this);
			}
		}
	}
}

void APTWSpawnItemVolume::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		if (UWorld* World = GetWorld())
		{
			if (UPTWItemSpawnManager* Manager = World->GetSubsystem<UPTWItemSpawnManager>())
			{
				Manager->UnregisterSpawnVolume(this);
			}
		}
	}
	Super::EndPlay(EndPlayReason);
}

FVector APTWSpawnItemVolume::GetRandomPointInVolume() const
{
	return UKismetMathLibrary::RandomPointInBoundingBox(SpawnBox->GetComponentLocation(), SpawnBox->GetScaledBoxExtent());
}
