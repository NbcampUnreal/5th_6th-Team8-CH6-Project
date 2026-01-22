// Fill out your copyright notice in the Description page of Project Settings.


#include "UGCN_WeaponFire.h"
#include "NiagaraFunctionLibrary.h"



bool UUGCN_WeaponFire::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (!MyTarget)
	{
		return false;
	}
	
	if (FireVFX)
	{
		
	}
	
	
	return true;
}
