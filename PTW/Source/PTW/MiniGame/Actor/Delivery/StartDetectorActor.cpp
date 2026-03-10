#include "StartDetectorActor.h"

#include "CoreFramework/PTWPlayerCharacter.h"
#include "MiniGame/GameMode/PTWDeliveryGameMode.h"


AStartDetectorActor::AStartDetectorActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AStartDetectorActor::OnDetectOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority())
	{
		if (APTWDeliveryGameMode* DeliveryGameMode = GetWorld()->GetAuthGameMode<APTWDeliveryGameMode>())
		{
			if (APTWPlayerCharacter* PC = Cast<APTWPlayerCharacter>(OtherActor))
			{
				DeliveryGameMode->GiveDeliveryItems(PC, EffectToApply);
			}
		}
	}
}


