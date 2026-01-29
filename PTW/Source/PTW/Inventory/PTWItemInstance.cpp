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
	DOREPLIFETIME(UPTWItemInstance, SpawnedWeapon1P);
	DOREPLIFETIME(UPTWItemInstance, SpawnedWeapon3P);
}

void UPTWItemInstance::OnRep_CurrentAmmo()
{
	
}

void UPTWItemInstance::OnRep_SpawnedWeapon()
{
	CurrentAmmo = SpawnedWeapon1P->GetWeaponData()->MaxAmmo;
}

void UPTWItemInstance::OnRep_SpawnedWeapon3P()
{
	
}

void UPTWItemInstance::SetCurrentAmmo(int32 NewAmmo)
{
	int32 MaxAmmo = GetMaxAmmo();
	// 값의 범위를 제한하고 변경된 경우에만 방송
	int32 ClampedAmmo = FMath::Clamp(NewAmmo, 0, MaxAmmo);

	if (CurrentAmmo != ClampedAmmo)
	{
		CurrentAmmo = ClampedAmmo;
		OnAmmoChanged.Broadcast(CurrentAmmo, MaxAmmo);
	}
}

int32 UPTWItemInstance::GetMaxAmmo()
{
	if (SpawnedWeapon1P && SpawnedWeapon1P->GetWeaponData())
	{
		return SpawnedWeapon1P->GetWeaponData()->MaxAmmo;
	}
	return 0;
}

