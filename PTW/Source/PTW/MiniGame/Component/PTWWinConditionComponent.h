// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWGameModeBaseComponent.h"
#include "MiniGame/PTWMiniGameRule.h"
#include "PTWWinConditionComponent.generated.h"


class IPTWPlayerRoundDataInterface;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PTW_API UPTWWinConditionComponent : public UPTWGameModeBaseComponent
{
	GENERATED_BODY()

public:	
	UPTWWinConditionComponent();

	/** 컴포넌트 초기화 함수 */
	void InitWinConditionComponent(APTWGameState* InGameState, const FPTWMiniGameRule* InMiniGameRule);
	
	/** 게임 종료 조건 체크 */
	void CheckEndGameCondition();
	/** 서바이벌 모드 승리 조건 체크 */
	void CheckSurvivalCondition();
	/** 목표 점수 승리 조건 체크 */
	void CheckTargetScoreCondition();
	
private:
	/** 마지막으로 사망한 플레이어 찾기 */
	IPTWPlayerRoundDataInterface* FindLastDeadPlayer();

private:
	const FPTWMiniGameRule* MiniGameRule;
};
