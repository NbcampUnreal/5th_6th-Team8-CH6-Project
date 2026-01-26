// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWItemInstance.h"

#include "PTWWeaponActor.h"
#include "PTWWeaponData.h"
#include "Net/UnrealNetwork.h"

void UPTWItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UPTWItemInstance, ItemDef);
	DOREPLIFETIME(UPTWItemInstance, CurrentAmmo);
	DOREPLIFETIME(UPTWItemInstance, SpawnedWeapon);
}

void UPTWItemInstance::OnRep_CurrentAmmo()
{
	
}

void UPTWItemInstance::OnRep_SpawnedWeapon()
{
	CurrentAmmo = SpawnedWeapon->GetWeaponData()->MaxAmmo;
}


