// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWItemInstance.h"

#include "../../Weapon/PTWWeaponActor.h"
#include "../../Weapon/PTWWeaponData.h"
#include "Net/UnrealNetwork.h"

void UPTWItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UPTWItemInstance, ItemDef);
}



