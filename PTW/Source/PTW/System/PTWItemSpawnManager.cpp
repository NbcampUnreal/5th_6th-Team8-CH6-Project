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
#include "Gameplay/PTWSpawnItemVolume.h"
#include "Gameplay/Pickup/PTWPickupCoin.h" 
#include "Gameplay/Pickup/PTWPickupWeapon.h"
#include "Kismet/KismetSystemLibrary.h"


void UPTWItemSpawnManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

const TCHAR* DTPath = TEXT("/Game/_PTW/Data/DT_ItemSpawnData.DT_ItemSpawnData");
	
	ItemSpawnTable = LoadObject<UDataTable>(nullptr, DTPath);

	if (!ItemSpawnTable)
	{
		UE_LOG(LogTemp, Error, TEXT("[PTWItemSpawnSubsystem] Failed to load DataTable at: %s"), DTPath);
	}

	FString Path = TEXT("/Game/_PTW/Blueprints/Gameplay/Pickup/BP_PTWPickupCoin.BP_PTWPickupCoin_C");
	FString WeaponPath = TEXT("/Game/_PTW/BluePrints/Gameplay/Pickup/BP_PTWPickupWeapon.BP_PTWPickupWeapon_C");

	CoinClass = LoadClass<APTWPickupCoin>(nullptr, *Path);
	PickupWeaponClass = LoadClass<APTWPickupWeapon>(nullptr, *WeaponPath);

	if (!CoinClass || !PickupWeaponClass)
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
	
	SpawnedWeapon1P->SetWeaponItemInstance(WeaponItemInst);
	SpawnedWeapon3P->SetWeaponItemInstance(WeaponItemInst);

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

void UPTWItemSpawnManager::RegisterSpawnVolume(APTWSpawnItemVolume* Volume)
{
	if (Volume && !SpawnVolumes.Contains(Volume))
	{
		SpawnVolumes.Add(Volume);
	}
}

void UPTWItemSpawnManager::UnregisterSpawnVolume(APTWSpawnItemVolume* Volume)
{
	SpawnVolumes.Remove(Volume);
}

void UPTWItemSpawnManager::SpawnCoinInRandomVolume()
{
	if (SpawnVolumes.Num() == 0 || !CoinClass) return;

	int32 RandIdx = FMath::RandRange(0, SpawnVolumes.Num() - 1);
	APTWSpawnItemVolume* TargetVolume = SpawnVolumes[RandIdx];
	if (!TargetVolume) return;

	UWorld* World = GetWorld();
	if (!World) return;

	FVector SpawnLocation = FVector::ZeroVector;
	bool bFoundValidLocation = false;

	for (int32 i = 0; i < 10; ++i)
	{
		FVector CandidateLoc = TargetVolume->GetRandomPointInVolume();

		TArray<AActor*> ActorsToIgnore;
		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_PhysicsBody));

		TArray<AActor*> ResultActors;

		bool bHit = UKismetSystemLibrary::SphereOverlapActors(
			World,
			CandidateLoc,
			40.0f,
			ObjectTypes,
			nullptr,
			ActorsToIgnore,
			ResultActors
		);

		if (!bHit)
		{
			SpawnLocation = CandidateLoc;
			bFoundValidLocation = true;
			break;
		}
	}

	if (bFoundValidLocation)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		World->SpawnActor<APTWPickupCoin>(CoinClass, SpawnLocation, FRotator::ZeroRotator, Params);
	}
}


void UPTWItemSpawnManager::DropWeaponSpawn(UPTWWeaponInstance* WeaponInstance)
{
	AActor* Owner = WeaponInstance->GetTypedOuter<AActor>();
	if (!Owner) return;
	
	FVector ForwardOffset = Owner->GetActorLocation() + (Owner->GetActorForwardVector() * 250.0f);
	FVector SpawnLocation = GetGroundLocation(ForwardOffset);
	SpawnLocation.Z += 10.0f;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	APTWPickupWeapon* SpawnWeaponActor = GetWorld()->SpawnActor<APTWPickupWeapon>(PickupWeaponClass, 
			SpawnLocation, 
			Owner->GetActorRotation(), 
			SpawnParams
		);
	
	if (SpawnWeaponActor && WeaponInstance)
	{
		SpawnWeaponActor->SetWeaponInstance(WeaponInstance);
		WeaponInstance->Rename(nullptr, SpawnWeaponActor); //Outer 재설정
		WeaponInstance->DestroySpawnedActors();
	}
}


void UPTWItemSpawnManager::AddPickupWeapon(UPTWWeaponInstance* ItemInstance, APTWPlayerCharacter* TargetPlayer)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = TargetPlayer;
	SpawnParams.Instigator = TargetPlayer;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	UPTWItemDefinition* ItemDefinition = ItemInstance->ItemDef;
	
	APTWWeaponActor* SpawnedWeapon1P = GetWorld()->SpawnActor<APTWWeaponActor>(ItemDefinition->WeaponClass, SpawnParams);
	APTWWeaponActor* SpawnedWeapon3P = GetWorld()->SpawnActor<APTWWeaponActor>(ItemDefinition->WeaponClass, SpawnParams);
	
	SpawnedWeapon1P->SetFirstPersonMode(true);
	SpawnedWeapon3P->SetFirstPersonMode(false);
	
	ItemInstance->SpawnedWeapon1P = SpawnedWeapon1P;
	ItemInstance->SpawnedWeapon3P = SpawnedWeapon3P;
	
	SpawnedWeapon1P->SetWeaponItemInstance(ItemInstance);
	SpawnedWeapon3P->SetWeaponItemInstance(ItemInstance);
	
	UPTWInventoryComponent* Inventory = TargetPlayer->GetInventoryComponent();
	
	Inventory->AddItem(ItemInstance);
	TargetPlayer->GetWeaponComponent()->AttachWeaponToSocket(SpawnedWeapon1P, SpawnedWeapon3P, ItemInstance->ItemDef->WeaponTag);
}

FVector UPTWItemSpawnManager::GetGroundLocation(FVector StartLocation)
{
	FHitResult HitResult;
	FVector EndLocation = StartLocation + (FVector::UpVector * -500.0f);
    
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetTypedOuter<AActor>()); 

	// LineTrace 실행
	if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, Params))
	{
		return HitResult.Location;
	}

	return StartLocation; 
}
