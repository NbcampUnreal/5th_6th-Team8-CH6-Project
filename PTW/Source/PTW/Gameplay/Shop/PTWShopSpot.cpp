// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Shop/PTWShopSpot.h"
#include "System/Shop/PTWShopSubsystem.h"

APTWShopSpot::APTWShopSpot()
{
	PrimaryActorTick.bCanEverTick = false;
	StandMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StandMesh"));
	RootComponent = StandMesh;
}

void APTWShopSpot::BeginPlay()
{
	Super::BeginPlay();
	if (UPTWShopSubsystem* Sys = GetWorld()->GetSubsystem<UPTWShopSubsystem>())
	{
		Sys->RegisterShopSpot(this);
	}
}

void APTWShopSpot::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		if (UPTWShopSubsystem* Sys = GetWorld()->GetSubsystem<UPTWShopSubsystem>())
		{
			Sys->UnregisterShopSpot(this);
		}
	}
	Super::EndPlay(EndPlayReason);
}

