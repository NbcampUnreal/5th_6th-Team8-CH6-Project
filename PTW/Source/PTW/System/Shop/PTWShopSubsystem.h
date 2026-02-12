// Fill out your copyright notice in the Description page of Project Settings.

// Source/PTW/System/Shop/PTWShopSubsystem.h

// Source/PTW/System/Shop/PTWShopSubsystem.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Gameplay/Shop/PTWShopItemData.h"
#include "GameplayTagContainer.h"
#include "PTWShopSubsystem.generated.h"

class APTWShopNPC;
class APTWShopSpot;

UCLASS()
class PTW_API UPTWShopSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	void RegisterShopSpot(APTWShopSpot* Spot);
	void UnregisterShopSpot(APTWShopSpot* Spot);

	/**
	 * 라운드 시작시 호출 상점 배치
	 * @param NextMinigameTag : 다음 미니게임 태그 (필터링용)
	 * @param RoundEventTag   : 이번 라운드 이벤트 태그 (가격 변동)
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop")
	void InitializeShopsForRound(FGameplayTag NextMinigameTag, FGameplayTag RoundEventTag);

	/* 라운드 이벤트에 따른 가격 배율이 적용된 최종 가격 반환 */
	UFUNCTION(BlueprintPure, Category = "Shop")
	int32 GetItemPrice(FName ItemID) const;

	const FShopItemRow* GetShopItemData(FName ItemID) const;

protected:
	TArray<EShopCategory> SelectShopCategories(int32 TargetCount);
	TArray<FName> SelectItemsForShop(EShopCategory Category, FGameplayTag BanTag);

	//현재 이벤트 태그에 따른 가격 배율 계산 (0.5 ~ 2.0)
	float GetPriceMultiplier() const;

protected:
	UPROPERTY()
	TObjectPtr<UDataTable> MasterShopTable;

	TMap<FName, FShopItemRow> CachedShopItems;

	UPROPERTY()
	TArray<TObjectPtr<APTWShopSpot>> ShopSpots;

	UPROPERTY()
	TArray<TObjectPtr<APTWShopNPC>> ActiveNPCs;

	UPROPERTY()
	TSubclassOf<APTWShopNPC> ShopNPCClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shop State")
	FGameplayTag CurrentRoundEventTag;
};
