// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/Actor/RedLight/PTWRedLightFinishBox.h"
#include "Components/BoxComponent.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "MiniGame/Character/RedLight/PTWRedLightCharacter.h"
#include "MiniGame/GameMode/PTWRedLightGameMode.h"

APTWRedLightFinishBox::APTWRedLightFinishBox()
{
	PrimaryActorTick.bCanEverTick = false;

	FinishVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("FinishVolume"));
	RootComponent = FinishVolume;

	FinishVolume->SetCollisionProfileName(TEXT("Trigger"));
}

void APTWRedLightFinishBox::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		FinishVolume->OnComponentBeginOverlap.AddDynamic(this, &APTWRedLightFinishBox::OnOverlapBegin);
	}
}

void APTWRedLightFinishBox::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || !OtherActor) return;

	if (OtherActor->IsA<APTWRedLightCharacter>()) return;

	if (APTWPlayerCharacter* Runner = Cast<APTWPlayerCharacter>(OtherActor))
	{
		if (APTWRedLightGameMode* GM = Cast<APTWRedLightGameMode>(GetWorld()->GetAuthGameMode()))
		{
			GM->PlayerFinished(Runner);
		}
	}
}

