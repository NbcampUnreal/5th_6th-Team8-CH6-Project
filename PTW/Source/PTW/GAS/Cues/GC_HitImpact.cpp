// Fill out your copyright notice in the Description page of Project Settings.


#include "GC_HitImpact.h"

#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

bool UGC_HitImpact::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	const FVector Location = Parameters.Location;
	const FRotator Rotation = Parameters.Normal.Rotation();
	
	if (ImpactFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ImpactFX,
			Location,
			Rotation
		);
	}
	
	if (ImpactSound)
	{
		APawn* InstPawn = Cast<APawn>(Parameters.Instigator.Get());
		APawn* TargetPawn = Cast<APawn>(MyTarget);
		
		bool bIsInstigator = false;
		bool bIsTarget = false;
		
		if (InstPawn && InstPawn->IsLocallyControlled())
		{
			bIsTarget = true;
		}
		
		if (TargetPawn && TargetPawn->IsLocallyControlled())
		{
			bIsTarget = true;
		}
		
		if (bIsTarget || bIsInstigator)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactSound, Location);
		}
	}
	
	return Super::OnExecute_Implementation(MyTarget, Parameters);
}
