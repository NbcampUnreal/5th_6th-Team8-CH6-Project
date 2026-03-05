// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MiniGame/PTWMiniGameMode.h"
#include "PTWGhostChaseMiniGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API APTWGhostChaseMiniGameMode : public APTWMiniGameMode
{
	GENERATED_BODY()
	
public:
	APTWGhostChaseMiniGameMode();

	/** 플레이어 사망 시 호출 (기존 PTWMiniGameMode 또는 Character에서 호출해줘야 함) */
	virtual void OnPlayerEliminated(AController* EliminatedController);

protected:
	virtual void BeginPlay() override;

	/* GameWaiting 단계: 10초 카운트다운 설정 */
	virtual void WaitingToStartRound() override;

	/* GameStart 단계: 3분 제한시간 및 게임 로직 활성화 */
	virtual void StartRound() override;

private:
	/* 플레이어들을 랜덤하게 섞고 원형 타겟 체인 생성 */
	void SetupTargetChain();

	/* 모든 생존 플레이어에게 투명화 적용 (나중에 GAS Tag로 교체 용이하게 분리) */
	void ApplyInvisibilityToAll();

	/* 특정 플레이어의 타겟이 변경되었음을 알림 (UI 갱신용) */
	void UpdatePlayerTargetUI(AController* Chaser, AController* NewTarget);

private:
	/* 현재 생존하여 게임에 참여 중인 플레이어 리스트 (순서가 타겟 체인임) */
	UPROPERTY()
	TArray<AController*> ActiveChasers;

	/* 타겟을 관리하는 맵 */
	UPROPERTY()
	TMap<AController*, AController*> TargetMap;
};
