// Fill out your copyright notice in the Description page of Project Settings.


#include "GC_WeaponFire.h"

#include "NiagaraFunctionLibrary.h"
#include "Inventory/PTWWeaponActor.h"


bool UGC_WeaponFire::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (!MyTarget)
	{
		return false;
	}
	UObject* SourceObj = Parameters.EffectContext.Get()->GetSourceObject();
	
	if (APTWWeaponActor* Weapon = Cast<APTWWeaponActor>(SourceObj))
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
		   FireVFX,
		   Weapon->GetMuzzleComponent(),
		   NAME_None,
		   FVector::ZeroVector,
		   FRotator::ZeroRotator, 
		   EAttachLocation::SnapToTargetIncludingScale,
		   true
		);
	}
	
	return true;
}
