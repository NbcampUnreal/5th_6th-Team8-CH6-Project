// Fill out your copyright notice in the Description page of Project Settings.


#include "GC_HitImpact.h"

#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "PTWGameplayTag/GameplayTags.h"

bool UGC_HitImpact::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	const FVector Location = Parameters.Location;
	const FRotator Rotation = Parameters.Normal.Rotation();
	
	APawn* TargetPawn = Cast<APawn>(MyTarget);
	bool bHitWall = Parameters.AggregatedSourceTags.HasTag(GameplayTags::GameplayCue::Hit::Wall);
	
	if (bHitWall)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			WallHitImpactFX,
			Location,
			Rotation
		);
	}
	else
	{
		if (TargetPawn && PlayerHitImpactFX)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				PlayerHitImpactFX,
				Location,
				Rotation
			);
		}
	}
	
	if (PlayerImpactSound)
	{
		APawn* InstPawn = Cast<APawn>(Parameters.Instigator.Get());
		
		bool bIsInstigator = false;
		bool bIsTarget = false;
		
		if (InstPawn && InstPawn->IsLocallyControlled())
		{
			bIsInstigator = true;
		}
		
		if (TargetPawn && TargetPawn->IsLocallyControlled())
		{
			bIsTarget = true;
		}
		
		if (bIsTarget || bIsInstigator)
		{
			if (!bHitWall) UGameplayStatics::PlaySoundAtLocation(GetWorld(), PlayerImpactSound, Location);
			else UGameplayStatics::PlaySoundAtLocation(GetWorld(), ETCImpactSound, Location);
		}
	}
	
	return Super::OnExecute_Implementation(MyTarget, Parameters);
}
