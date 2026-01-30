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
	//Fix 박태웅(01.29) - (크래시방지)
	if (!SpawnedWeapon1P || !SpawnedWeapon3P)
	{
		if (SpawnedWeapon1P) SpawnedWeapon1P->Destroy();
		if (SpawnedWeapon3P) SpawnedWeapon3P->Destroy();
		return;
	}
	
	SpawnedWeapon1P->SetFirstPersonMode(true);
	SpawnedWeapon3P->SetFirstPersonMode(false);

	//Fix 박태웅(01.29) - (업데이트 시도)
	SpawnedWeapon1P->ForceNetUpdate();
	SpawnedWeapon3P->ForceNetUpdate();

	WeaponItemInst->ItemDef = ItemDefinition;
	WeaponItemInst->SpawnedWeapon1P = SpawnedWeapon1P;
	WeaponItemInst->SpawnedWeapon3P = SpawnedWeapon3P;

	//Fix 박태웅(01.29) - (데이터 가져오기)
	if (const UPTWWeaponData* WData = SpawnedWeapon1P->GetWeaponData())
	{
		WeaponItemInst->CurrentAmmo = WData->MaxAmmo;
	}
	else
	{
		WeaponItemInst->CurrentAmmo = 0;
		UE_LOG(LogTemp, Warning, TEXT("SpawnWeaponActor: WeaponData is null for %s"), *GetNameSafe(ItemDefinition));
	}

	Inventory->AddItem(WeaponItemInst);
	TargetPlayer->AttachWeaponToSocket(SpawnedWeapon1P, SpawnedWeapon3P, WeaponTag);
}
