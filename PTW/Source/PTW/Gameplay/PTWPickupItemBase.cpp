// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/PTWPickupItemBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

APTWPickupItemBase::APTWPickupItemBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	RootComponent = SphereComp;
	SphereComp->SetSphereRadius(50.0f);
	SphereComp->SetCollisionProfileName(TEXT("Trigger"));

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetCollisionProfileName(TEXT("NoCollision"));

	RotationComp = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotationComp"));
	RotationComp->RotationRate = FRotator(0.0f, 90.0f, 0.0f);
}

void APTWPickupItemBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SphereComp->OnComponentBeginOverlap.AddDynamic(this, &APTWPickupItemBase::OnSphereOverlap);
	}
}

void APTWPickupItemBase::OnSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APTWPlayerCharacter* PlayerCharacter = Cast<APTWPlayerCharacter>(OtherActor);
	if (PlayerCharacter)
	{
		OnPickedUp(PlayerCharacter);

		if (PickupSound)
		{
			UGameplayStatics::PlaySound2D(GetWorld(), PickupSound);;
		}
		if (PickupVFX)
		{
			UGameplayStatics::SpawnEmitterAtLocation(this, PickupVFX, GetActorLocation());
		}

		Destroy();
	}
}

void APTWPickupItemBase::OnPickedUp(APTWPlayerCharacter* Player)
{

}
