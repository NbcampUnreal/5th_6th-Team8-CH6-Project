#include "StartDetector.h"
#include "Components/BoxComponent.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "MiniGame/GameMode/PTWDeliveryGameMode.h"

AStartDetector::AStartDetector()
{
	PrimaryActorTick.bCanEverTick = false;
	
	DetectArea = CreateDefaultSubobject<UBoxComponent>(TEXT("DetectArea"));
	DetectArea->SetupAttachment(RootComponent);
	DetectArea->OnComponentBeginOverlap.AddDynamic(this, &AStartDetector::OnDetectPlayer);
}

void AStartDetector::OnDetectPlayer(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, 
	bool bFromSweep, 
	const FHitResult& SweepResult)
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


