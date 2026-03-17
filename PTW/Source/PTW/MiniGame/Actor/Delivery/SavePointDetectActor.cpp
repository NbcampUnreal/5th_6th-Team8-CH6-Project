#include "SavePointDetectActor.h"

#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/PTWPlayerController.h"
#include "MiniGame/GameMode/PTWDeliveryGameMode.h"


ASavePointDetectActor::ASavePointDetectActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASavePointDetectActor::OnDetectOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority())
	{
		if (APTWDeliveryGameMode* DeliveryGameMode = GetWorld()->GetAuthGameMode<APTWDeliveryGameMode>())
		{
			if (APTWPlayerCharacter* PC = Cast<APTWPlayerCharacter>(OtherActor))
			{
				APTWPlayerController* PCController = Cast<APTWPlayerController>(PC->GetController());
				if (!PCController) return;
				DeliveryGameMode->SetPlayerSpawnLocation(PCController, GetActorLocation());
			}
		}
	}
}



