#include "GoalDetector.h"

#include "Components/BoxComponent.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "MiniGame/GameMode/PTWDeliveryGameMode.h"

AGoalDetector::AGoalDetector()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AGoalDetector::OnDetectPlayer(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority())
	{
		if (APTWDeliveryGameMode* DeliveryGameMode = Cast<APTWDeliveryGameMode>(GetWorld()->GetAuthGameMode()))
		{
			if (APTWPlayerCharacter* PC = Cast<APTWPlayerCharacter>(OtherActor))
			{
				DeliveryGameMode->GoalPlayer(PC);
			}
		}
	}
}


