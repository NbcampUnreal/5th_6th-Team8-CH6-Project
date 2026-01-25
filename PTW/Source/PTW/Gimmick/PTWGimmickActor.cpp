// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGimmickActor.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"

APTWGimmickActor::APTWGimmickActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->SetupAttachment(RootComponent);
	Collision->SetSphereRadius(60.f);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void APTWGimmickActor::BeginPlay()
{
	Super::BeginPlay();
	Collision->OnComponentBeginOverlap.AddDynamic(this, &APTWGimmickActor::OnOverlapBegin);
}

void APTWGimmickActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;
	if (bConsumed) return;
	if (!OtherActor || OtherActor == this) return;

	if (!Cast<ACharacter>(OtherActor)) return;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (!ASC) return;

	bConsumed = true;
	Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	FGameplayEventData EventData;
	EventData.EventTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Gimmick.Collect"));
	EventData.Instigator = OtherActor;
	EventData.Target = OtherActor;
	EventData.OptionalObject = this; 

	ASC->HandleGameplayEvent(EventData.EventTag, &EventData);
}

