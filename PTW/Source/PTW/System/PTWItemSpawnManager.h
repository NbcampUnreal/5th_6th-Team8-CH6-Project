// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "PTWItemSpawnManager.generated.h"

class UPTWItemDefinition;
class APTWWeaponActor;
class APTWPlayerCharacter;
class UDataTable;
class APTWPlayerState;
class APTWPickupCoin;
class APTWSpawnItemVolume;

/**
 * 
 */
UCLASS()
class PTW_API UPTWItemSpawnManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable)
	void SpawnWeaponActor(APTWPlayerCharacter* TargetPlayer, UPTWItemDefinition* ItemDefinition, FGameplayTag WeaponTag);
	
	/** * 저장된 ID 목록을 순회하며 실제 인스턴스를 생성해 인벤토리에 지급
	 * @param PS : 아이템을 지급받을 대상 플레이어 스테이트
	 */
	UFUNCTION(BlueprintCallable)
	void SpawnAndGiveItems(APTWPlayerState* PS);


	/* 특정 ItemDefinition을 기반으로 아이템을 생성하여 플레이어에게 지급 */
	UFUNCTION(BlueprintCallable, Category = "ItemSpawn")
	void SpawnSingleItem(APTWPlayerState* PS, UPTWItemDefinition* ItemDef);

	void RegisterSpawnVolume(class APTWSpawnItemVolume* Volume);
	void UnregisterSpawnVolume(class APTWSpawnItemVolume* Volume);

	UFUNCTION(BlueprintCallable)
	void SpawnCoinInRandomVolume();
protected:
	/** 스폰 데이터 테이블 (RowName: ItemID, Value: ItemDefinition) */
	UPROPERTY()
	TObjectPtr<UDataTable> ItemSpawnTable;

	UPROPERTY()
	TArray<TObjectPtr<class APTWSpawnItemVolume>> SpawnVolumes;

	UPROPERTY(EditDefaultsOnly, Category = "Coin")
	TSubclassOf<class APTWPickupCoin> CoinClass;

};
