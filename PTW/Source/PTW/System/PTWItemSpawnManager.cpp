// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWItemSpawnManager.h"

#include "Inventory/PTWInventoryComponent.h"
#include "Inventory/PTWItemDefinition.h"
#include "Inventory/Instance/PTWItemInstance.h"
#include "Inventory/Instance/PTWWeaponInstance.h"
#include "Inventory/Instance/PTWActiveItemInstance.h"
#include "Inventory/Instance/PTWPassiveItemInstance.h"
#include "Weapon/PTWWeaponActor.h"
#include "Weapon/PTWWeaponData.h"
#include "PTW/CoreFramework/PTWPlayerCharacter.h"
#include "PTW/CoreFramework/Character/Component/PTWWeaponComponent.h"
#include "System/Shop/PTWItemSpawnData.h"
#include "CoreFramework/PTWPlayerState.h"


void UPTWItemSpawnManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

const TCHAR* DTPath = TEXT("/Game/_PTW/Data/DT_ItemSpawnData.DT_ItemSpawnData");
	
	ItemSpawnTable = LoadObject<UDataTable>(nullptr, DTPath);

	if (!ItemSpawnTable)
	{
		UE_LOG(LogTemp, Error, TEXT("[PTWItemSpawnSubsystem] Failed to load DataTable at: %s"), DTPath);
	}
}

void UPTWItemSpawnManager::SpawnWeaponActor(APTWPlayerCharacter* TargetPlayer, UPTWItemDefinition* ItemDefinition, FGameplayTag WeaponTag)
{
	UWorld* World = GetWorld();

	if (!World || World->GetNetMode() == NM_Client)
	{
		return;
	}
	
	UPTWInventoryComponent* Inventory = TargetPlayer->GetInventoryComponent();
	if (!Inventory) return;
	
	UPTWWeaponInstance* WeaponItemInst = NewObject<UPTWWeaponInstance>(Inventory);
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
	TargetPlayer->GetWeaponComponent()->AttachWeaponToSocket(SpawnedWeapon1P, SpawnedWeapon3P, WeaponTag);
}

//TODO : 무기 및 액티브 중복체크, 리팩토링
void UPTWItemSpawnManager::SpawnAndGiveItems(APTWPlayerState* PS)
{
	if (!PS || !ItemSpawnTable) return;

	APawn* PlayerPawn = PS->GetPawn();
	APTWPlayerCharacter* PlayerChar = Cast<APTWPlayerCharacter>(PlayerPawn);

	if (!PlayerChar)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SpawnSystem] PlayerPawn is NULL for PlayerState: %s (Too early?)"), *PS->GetPlayerName());
		return;
	}
	UPTWInventoryComponent* InventoryComp = PlayerPawn->FindComponentByClass<UPTWInventoryComponent>();
	if (!InventoryComp)
	{
		UE_LOG(LogTemp, Error, TEXT("[SpawnSystem] Inventory Component Not Found on Character: %s"), *PlayerChar->GetName());
		return;
	}

	TArray<FString> IDsToSpawn = PS->GetPlayerData().InventoryItemIDs;

	UE_LOG(LogTemp, Log, TEXT("[SpawnSystem] Start Spawning %d Items for %s"), IDsToSpawn.Num(), *PS->GetPlayerName());

	for (const FString& IDStr : IDsToSpawn)
	{
		FName RowName = FName(*IDStr);
		static const FString ContextString(TEXT("ItemSpawn"));

		FPTWItemSpawnRow* Row = ItemSpawnTable->FindRow<FPTWItemSpawnRow>(RowName, ContextString);

		if (Row && !Row->ItemDefinition.IsNull())
		{
			UPTWItemDefinition* LoadedDef = Row->ItemDefinition.LoadSynchronous();

			if (LoadedDef)
			{
				if (LoadedDef->ItemType == EItemType::Weapon)
				{
					SpawnWeaponActor(PlayerChar, LoadedDef, LoadedDef->WeaponTag);

					UE_LOG(LogTemp, Log, TEXT("   -> Spawned Weapon Actor & Instance: %s (Tag: %s)"),
						*IDStr, *LoadedDef->WeaponTag.ToString());
				}
				else
				{
					UClass* InstanceClassToSpawn = nullptr;

					switch (LoadedDef->ItemType)
					{
					case EItemType::Active:
						InstanceClassToSpawn = UPTWActiveItemInstance::StaticClass();
						break;

					case EItemType::Passive:
						InstanceClassToSpawn = UPTWPassiveItemInstance::StaticClass();
						break;

					default:
						InstanceClassToSpawn = UPTWItemInstance::StaticClass();
						break;
					}
					if (InstanceClassToSpawn)
					{
						UPTWItemInstance* NewItem = NewObject<UPTWItemInstance>(InventoryComp, InstanceClassToSpawn);

						if (NewItem)
						{
							NewItem->ItemDef = LoadedDef;

							InventoryComp->AddItem(NewItem);

							UE_LOG(LogTemp, Log, TEXT("   -> Spawned: %s (Type: %s, Class: %s)"),
								*IDStr,
								*UEnum::GetValueAsString(LoadedDef->ItemType),
								*InstanceClassToSpawn->GetName());
						}
					}
				} 
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("   -> Failed to load ItemDefinition for ID: %s"), *IDStr);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("   -> ItemID not found in DT: %s"), *IDStr);
		}
	}
}


void UPTWItemSpawnManager::SpawnSingleItem(APTWPlayerState* PS, UPTWItemDefinition* ItemDef)
{
	if (!PS || !ItemDef) return;

	APawn* PlayerPawn = PS->GetPawn();
	APTWPlayerCharacter* PlayerChar = Cast<APTWPlayerCharacter>(PlayerPawn);
	if (!PlayerChar)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SpawnSingleItem] Character is Invalid for Player: %s"), *PS->GetPlayerName());
		return;
	}

	UPTWInventoryComponent* InventoryComp = PlayerChar->GetInventoryComponent();
	if (!InventoryComp)
	{
		UE_LOG(LogTemp, Error, TEXT("[SpawnSingleItem] Inventory Component Missing!"));
		return;
	}

	if (ItemDef->ItemType == EItemType::Weapon)
	{
		SpawnWeaponActor(PlayerChar, ItemDef, ItemDef->WeaponTag);

		UE_LOG(LogTemp, Log, TEXT("[SpawnSingleItem] Spawned Weapon: %s"), *ItemDef->GetName());
	}
	else
	{
		UClass* InstanceClassToSpawn = nullptr;

		switch (ItemDef->ItemType)
		{
		case EItemType::Active:
			InstanceClassToSpawn = UPTWActiveItemInstance::StaticClass();
			break;
		case EItemType::Passive:
			InstanceClassToSpawn = UPTWPassiveItemInstance::StaticClass();
			break;
		default:
			InstanceClassToSpawn = UPTWItemInstance::StaticClass();
			break;
		}

		if (InstanceClassToSpawn)
		{
			UPTWItemInstance* NewItem = NewObject<UPTWItemInstance>(InventoryComp, InstanceClassToSpawn);
			if (NewItem)
			{
				NewItem->ItemDef = ItemDef;
				InventoryComp->AddItem(NewItem);

				UE_LOG(LogTemp, Log, TEXT("[SpawnSingleItem] Spawned Instance: %s (Type: %s)"),
					*ItemDef->GetName(), *UEnum::GetValueAsString(ItemDef->ItemType));
			}
		}
	}
}
