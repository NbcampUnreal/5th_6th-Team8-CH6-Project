// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTW/CoreFramework/Game/GameMode/PTWGameMode.h"
#include "PTWMiniGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API APTWMiniGameMode : public APTWGameMode
{
	GENERATED_BODY()

public:
	APTWMiniGameMode();

	/** 플레이어의 승점을 증가시키는 함수
	* - 특정 Pawn에 대응되는 플레이어 데이터에 승점을 누적
	*
	* @param PointPawn 승점을 획득한 플레이어의 Pawn
	* @param AddPoint 추가할 승점 값
	*/
	UFUNCTION(BlueprintCallable)
	void AddWinPoint(APawn* PointPawn, int32 AddPoint);
	
protected:
	virtual void BeginPlay() override;
	virtual void EndTimer() override;

	/** 미니게임 진행 시간 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Game|Timer")
	float MiniGameTime = 90;
};
