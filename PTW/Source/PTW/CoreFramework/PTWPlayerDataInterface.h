// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PTWPlayerDataInterface.generated.h"

class APTWPlayerState;
// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPTWPlayerDataInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PTW_API IPTWPlayerDataInterface
{
	GENERATED_BODY()

	
public:
	virtual void SetDeathOrder(int32 Order)=0;
	/** 팀 설정 */
	virtual void SetTeamId(int32 Id) = 0;
	virtual int32 GetDeathOrder() const =0;
	virtual int32 GetTeamId() const = 0;
	/** 라운드 킬 증가 */
	virtual void AddKillCount(int32 Amount = 1) =0;
	
	/** 라운드 데스 증가 */
	virtual void AddDeathCount(int32 Amount = 1) =0;
	
	/** 라운드 점수 증가 */
	virtual void AddScore(int32 Amount) = 0;
	
	/** 라운드 더어터 초기화 */
	virtual void ResetRoundData() = 0;
	
	/** 승리 예상 선수 투표 */
	virtual void VotePredictedPlayer(FUniqueNetIdRepl PredictedPlayer) = 0;
};
