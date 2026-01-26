// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWItemSpawnManager.h"

#include "Inventory/PTWInventoryComponent.h"
#include "Inventory/PTWItemDefinition.h"
#include "Inventory/PTWItemInstance.h"
#include "Inventory/PTWWeaponActor.h"
#include "Inventory/PTWWeaponData.h"
#include "PTW/CoreFramework/PTWPlayerCharacter.h"


void UPTWItemSpawnManager::SpawnWeaponActor(APTWPlayerCharacter* TargetPlayer, UPTWItemDefinition* ItemDefinition)
{
	UWorld* World = GetWorld();

	if (!World || World->GetNetMode() == NM_Client)
	{
		return;
	}
	
	UPTWInventoryComponent* Inventory = TargetPlayer->GetInventoryComponent();
	if (!Inventory) return;
	
	UPTWItemInstance* WeaponItemInst = NewObject<UPTWItemInstance>(Inventory);
	if (!WeaponItemInst) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = TargetPlayer;
	SpawnParams.Instigator = TargetPlayer;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APTWWeaponActor* SpawnedWeapon = GetWorld()->SpawnActor<APTWWeaponActor>(ItemDefinition->WeaponClass, SpawnParams);
	if (!SpawnedWeapon) return;
	
	
	WeaponItemInst->ItemDef = ItemDefinition;
	WeaponItemInst->SpawnedWeapon = SpawnedWeapon;
	WeaponItemInst->CurrentAmmo = SpawnedWeapon->GetWeaponData()->MaxAmmo;
	Inventory->AddItem(ItemDefinition, SpawnedWeapon);
	//SpawnedWeapon->AttachToComponent(TargetPlayer->GetMesh1P(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("WeaponSocket"));
}
