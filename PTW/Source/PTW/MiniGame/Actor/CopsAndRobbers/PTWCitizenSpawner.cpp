// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWCitizenSpawner.h"
#include "NavigationSystem.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "MiniGame/Character/AI/CopsAndRobbers/PTWCARCitizen.h"

APTWCitizenSpawner::APTWCitizenSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	
	SpawnVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnVolume"));
	RootComponent = SpawnVolume;
	
	SpawnVolume->SetBoxExtent(FVector(1000.0f, 1000.0f, 200.0f));
	SpawnVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	SpawnCount = 5;
}

void APTWCitizenSpawner::SpawningRandomLocation()
{
	if (!IsValid(CitizenClass)) return;

	UNavigationSystemV1* NavigationSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!IsValid(NavigationSystem)) return;
	
	FVector Origin = SpawnVolume->Bounds.Origin;
	FVector BoxExtent = SpawnVolume->Bounds.BoxExtent;

	for (int32 i = 0; i < SpawnCount; ++i)
	{
		FVector RandomPoint = UKismetMathLibrary::RandomPointInBoundingBox(Origin, BoxExtent);

		FNavLocation ProjectedLocation;
		if (NavigationSystem->ProjectPointToNavigation(RandomPoint, ProjectedLocation))
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			
			FVector SpawnLocation = ProjectedLocation.Location;
			if (APTWCARCitizen* DefaultCitizen = CitizenClass->GetDefaultObject<APTWCARCitizen>())
			{
				if (UCapsuleComponent* CapsuleComp = DefaultCitizen->GetCapsuleComponent())
				{
					SpawnLocation.Z += CapsuleComp->GetScaledCapsuleHalfHeight() + 5.0f;
				}
			}
			
			FRotator SpawnRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
			if (APTWCARCitizen* CitizenInstance = GetWorld()->SpawnActor<APTWCARCitizen>(
				CitizenClass, SpawnLocation, SpawnRotation, SpawnParams))
			{
				CitizenInstances.Add(CitizenInstance);
			}
		}
	}
}

void APTWCitizenSpawner::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		SpawningRandomLocation();
	}
}

void APTWCitizenSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APTWCitizenSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CitizenInstances.Empty();
	
	Super::EndPlay(EndPlayReason);
}

