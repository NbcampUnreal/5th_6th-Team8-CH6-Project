// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWCitizenSpawner.h"
#include "NavigationSystem.h"
#include "Components/BoxComponent.h"
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

	// 스폰 볼륨의 중심점(Origin)과 크기(BoxExtent) 가져오기
	FVector Origin = SpawnVolume->Bounds.Origin;
	FVector BoxExtent = SpawnVolume->Bounds.BoxExtent;

	for (int32 i = 0; i < SpawnCount; ++i)
	{
		// 1. 박스 범위 내에서 완전 랜덤한 3D 좌표 하나를 뽑습니다.
		FVector RandomPoint = UKismetMathLibrary::RandomPointInBoundingBox(Origin, BoxExtent);

		FNavLocation ProjectedLocation;
		// 2. 뽑은 랜덤 좌표 근처의 가장 가까운 내비메시(바닥) 좌표를 찾습니다.
		// 이 과정을 거쳐야 NPC가 공중에 스폰되거나 벽에 끼이는 버그를 막을 수 있습니다.
		if (NavigationSystem->ProjectPointToNavigation(RandomPoint, ProjectedLocation))
		{
			FActorSpawnParameters SpawnParams;
			// 스폰 시 약간의 충돌이 있어도 강제로 스폰하고 위치를 조정하도록 설정
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			// 3. 계산된 안전한 위치에 NPC 스폰!
			FVector SpawnLocation = ProjectedLocation.Location;
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

