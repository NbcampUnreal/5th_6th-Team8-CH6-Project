// Fill out your copyright notice in the Description page of Project Settings.


#include "StartDetector.h"

#include "Components/BoxComponent.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "MiniGame/GameMode/PTWDeliveryGameMode.h"


// Sets default values
AStartDetector::AStartDetector()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AStartDetector::OnDetectPlayer(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority())
	{
		if (APTWDeliveryGameMode* DeliveryGameMode = Cast<APTWDeliveryGameMode>(GetWorld()->GetAuthGameMode()))
		{
			if (APTWPlayerCharacter* PC = Cast<APTWPlayerCharacter>(OtherActor))
			{
				DeliveryGameMode->GiveDeliveryItems(PC);
			}
		}
	}
}


