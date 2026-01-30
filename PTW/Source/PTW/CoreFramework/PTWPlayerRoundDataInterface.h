// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PTWPlayerRoundDataInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPTWPlayerRoundDataInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PTW_API IPTWPlayerRoundDataInterface
{
	GENERATED_BODY()

	
public:
	/** 라운드 킬 증가 */
	virtual void AddKillCount(int32 Amount = 1);
	
	/** 라운드 데스 증가 */
	virtual void AddDeathCount(int32 Amount = 1);
	
	/** 라운드 점수 증가 */
	virtual void AddScore(int32 Amount);
	
	/** 라운드 더어터 초기화 */
	virtual void ResetRoundData() = 0;
};
