// Fill out your copyright notice in the Description page of Project Settings.


#include "GC_Explosion.h"

#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

bool UGC_Explosion::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	FVector Location = Parameters.Location;
	
	if (ExplosionVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplosionVFX, Location);
	}
	
	if (ExplosionSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSFX, Location);
	}
	
	return Super::OnExecute_Implementation(MyTarget, Parameters);
}
