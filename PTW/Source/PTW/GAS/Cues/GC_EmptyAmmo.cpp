// Fill out your copyright notice in the Description page of Project Settings.


#include "GC_EmptyAmmo.h"

#include "CoreFramework/PTWPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"


bool UGC_EmptyAmmo::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	APTWPlayerCharacter* PC = Cast<APTWPlayerCharacter>(MyTarget);
	if (!PC) return false;
	
	if (PC->IsLocallyControlled())
	{
		UGameplayStatics::PlaySoundAtLocation(
				GetWorld(), 
				EmptyAmmoSound, 
				PC->GetActorLocation()
		);
	}
	
	return true;
}
