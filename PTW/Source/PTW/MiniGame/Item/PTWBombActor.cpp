// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWBombActor.h"

#include "AbilitySystemComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

APTWBombActor::APTWBombActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// Collision 
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(Collision);
	Collision->InitSphereRadius(50.f);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Mesh
	BombMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BombMesh"));
	BombMesh->SetupAttachment(RootComponent);
	BombMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// GAS
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

void APTWBombActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (AbilitySystemComponent && BombStateTag.IsValid())
	{
		AbilitySystemComponent->AddLooseGameplayTag(BombStateTag);
	}
}

UAbilitySystemComponent* APTWBombActor::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}


