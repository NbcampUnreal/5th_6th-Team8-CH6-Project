// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWItemSpawnManager.h"
#include "Inventory/PTWInventoryComponent.h"
#include "Inventory/PTWItemDefinition.h"
#include "Inventory/PTWItemInstance.h"
#include "Inventory/PTWWeaponActor.h"
#include "Inventory/PTWWeaponData.h"
#include "PTW/CoreFramework/PTWPlayerCharacter.h"


void UPTWItemSpawnManager::SpawnWeaponActor(APTWPlayerCharacter* TargetPlayer, UPTWItemDefinition* ItemDefinition, FGameplayTag WeaponTag)
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

	APTWWeaponActor* SpawnedWeapon1P = GetWorld()->SpawnActor<APTWWeaponActor>(ItemDefinition->WeaponClass, SpawnParams);
	APTWWeaponActor* SpawnedWeapon3P = GetWorld()->SpawnActor<APTWWeaponActor>(ItemDefinition->WeaponClass, SpawnParams);
	if (!SpawnedWeapon1P && !SpawnedWeapon3P) return;
	
	
	SpawnedWeapon1P->bIsFirstPersonWeapon = true;
	SpawnedWeapon3P->bIsFirstPersonWeapon = false;

	WeaponItemInst->ItemDef = ItemDefinition;
	WeaponItemInst->SpawnedWeapon1P = SpawnedWeapon1P;
	WeaponItemInst->SpawnedWeapon3P = SpawnedWeapon3P;
	WeaponItemInst->CurrentAmmo = SpawnedWeapon1P->GetWeaponData()->MaxAmmo;
	Inventory->AddItem(ItemDefinition, SpawnedWeapon1P, SpawnedWeapon3P);
	TargetPlayer->AttachWeaponToSocket(SpawnedWeapon1P, SpawnedWeapon3P, WeaponTag);
}
