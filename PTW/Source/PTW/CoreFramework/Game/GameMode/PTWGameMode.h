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

	/** 플레이어가 서버에서 로그아웃할 때 호출
	* - 플레이어 수 갱신 및 게임 진행 상태 체크
	*/
	virtual void Logout(AController* Exiting) override;
	
	/** 특정 시간 동안 타이머를 시작
	 * @param TimeDuration 타이머 지속 시간(초)
	 */
	void StartTimer(float TimeDuration);
	
	/** 지정한 레벨로 이동 */
	UFUNCTION()
	void TravelLevel();

	/** 
	* 이동할 레벨 이름
	* - TravelLevel() 호출 시 어떤 레벨로 이동할지 지정
	* - 런타임에 다른 레벨로 전환할 때 사용
	*/
	FString TravelLevelName;
	
	/**
	 * 현재 게임의 상태를 관리하는 GameState 참조
	 * - 플레이어 목록 및 게임 진행 정보 접근에 사용
	 */
	UPROPERTY()
	TObjectPtr<APTWGameState> PTWGameState;
private:
	/** 타이머 갱신 처리
	 * - 타이머 종료 시 이벤트 호출
	 */
	void UpdateTimer();
	
	/** 내부 타이머 핸들 */
	FTimerHandle TimerHandle;
};
