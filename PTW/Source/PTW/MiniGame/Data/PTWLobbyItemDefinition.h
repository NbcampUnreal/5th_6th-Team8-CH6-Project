// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PTWLobbyItemDefinition.generated.h"


UENUM(BlueprintType)
enum class ELobbyItemType : uint8
{
	SavingGold, // 적금
	RandomActive, // 랜덤 액티브 추가
	RandomPassive, // 랜덤 패시브 추가
	GambleBox, // 랜던 골드 부여
	PredictionWin // 승리 예측
};

UCLASS()
class PTW_API UPTWLobbyItemDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	ELobbyItemType ItemType;

	/** 몇 라운드 후에 적금 골드를 받을 지 설정 **/
	UPROPERTY(EditDefaultsOnly, meta = (EditCondition= "ItemType == ELobbyItemType::SavingGold"))
	int32 DelayRound = 3;

	/** 적금 몇 골드 받을 지 설정 */
	UPROPERTY(EditDefaultsOnly, meta = (EditCondition= "ItemType == ELobbyItemType::SavingGold"))
	int32 RewardAmount = 1000;

	/** 승리 예측 적중 시 받을 골드 */
	UPROPERTY(EditDefaultsOnly, meta = (EditCondition= "ItemType == ELobbyItemType::PredictionWin"))
	int32 PredictionReward = 300;
};
