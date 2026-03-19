#include "SavePointDetectActor.h"

#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/PTWPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "MiniGame/GameMode/PTWDeliveryGameMode.h"
#include "Sound/SoundCue.h"


ASavePointDetectActor::ASavePointDetectActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASavePointDetectActor::OnDetectOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APTWPlayerCharacter* PC = Cast<APTWPlayerCharacter>(OtherActor);
	if (!PC) return;	
	
	if (HasAuthority())
	{
		if (APTWDeliveryGameMode* DeliveryGameMode = GetWorld()->GetAuthGameMode<APTWDeliveryGameMode>())
		{
			APTWPlayerController* PCController = Cast<APTWPlayerController>(PC->GetController());
			if (!PCController) return;
			DeliveryGameMode->SetPlayerSpawnLocation(PCController, GetActorLocation());
			DeliveryGameMode->ApplyGameEffect(PC, EffectToApply);
		}
	}
	SpeedUpPlaySound(PC);
}

void ASavePointDetectActor::SpeedUpPlaySound(APTWPlayerCharacter* TargetCharacter)
{
	UGameplayStatics::PlaySoundAtLocation(
		this,
		SpeedUpSound,
		TargetCharacter->GetActorLocation());
}



