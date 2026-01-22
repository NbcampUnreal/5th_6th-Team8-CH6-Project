// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "PTWGameMode.generated.h"

/**
 * 게임의 전체 흐름과 시작 조건을 관리하는 GameMode
 * - 플레이어 입장 관리
 * - 최소 인원 충족 여부 확인
 * - 대기 시간 이후 게임 시작 처리
 */
class APTWGameState;

UCLASS()
class PTW_API APTWGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	APTWGameMode();
protected:
	/**
	 * 게임 월드가 시작될 때 호출
	 * - 게임 시작을 위한 초기 설정 수행
	 * - GameState 참조 캐싱
	 */
	virtual void BeginPlay() override;
	
	/**
	 * 새로운 플레이어가 서버에 로그인했을 때 호출
	 * - 현재 접속 인원 갱신
	 * - 최소 시작 인원 충족 여부 체크
	 */
	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;

	void TravelLevel();
	
	/**
	 * 현재 게임의 상태를 관리하는 GameState 참조
	 * - 플레이어 목록 및 게임 진행 정보 접근에 사용
	 */
	UPROPERTY()
	TObjectPtr<APTWGameState> PTWGameState;
private:
	

	


	//FTimerHandle WaitingTimerHandle;
};
