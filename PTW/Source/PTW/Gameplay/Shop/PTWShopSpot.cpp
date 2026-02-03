// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Shop/PTWShopSpot.h"
#include "System/Shop/PTWShopSubsystem.h"
#include "Components/ArrowComponent.h"

APTWShopSpot::APTWShopSpot()
{
	PrimaryActorTick.bCanEverTick = false;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	SetRootComponent(RootScene);

	VisualizationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualizationMesh"));
	VisualizationMesh->SetupAttachment(RootScene);
	VisualizationMesh->SetHiddenInGame(true);
	VisualizationMesh->SetCollisionProfileName(TEXT("NoCollision"));
	VisualizationMesh->SetCastShadow(false);

	SpawnDirectionArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("SpawnDirectionArrow"));
	SpawnDirectionArrow->SetupAttachment(RootScene);
	SpawnDirectionArrow->ArrowSize = 1.0f;
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

