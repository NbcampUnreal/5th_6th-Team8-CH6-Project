// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "PTWGameMode.generated.h"


class APTWGameState;

/** 게임의 전체 흐름과 시작 조건을 관리하는 GameMode
* - 플레이어 입장/퇴장 관리
* - 최소 인원 및 대기 시간 기반 시작 처리
* - 타이머 및 레벨 트래블 연계
*/
UCLASS()
class PTW_API APTWGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	
	APTWGameMode();

protected:
	/** 게임 월드 시작 시 초기 설정 및 GameState 참조 캐싱 */
	virtual void InitGameState() override;
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void BeginPlay() override;

	/** 플레이어 로그인 시 호출(접속 인원/시작 조건 갱신) */
	virtual void PostLogin(APlayerController* NewPlayer) override;

	/** 플레이어 로그아웃 시 호출(접속 인원/게임 진행 상태 갱신) */
	virtual void Logout(AController* Exiting) override;

	/** 지정한 시간(초) 기준으로 타이머를 시작 */
	void StartTimer(float TimeDuration);

	/** 타이머 종료 시 호출(타이머 정리 및 종료 후 처리 트리거) */
	UFUNCTION()
	virtual void EndTimer();

	/** 설정된 TravelLevelName으로 레벨 이동 처리 */
	void TravelLevel();

	// 이동할 레벨 이름(TravelLevel에서 사용)
	FString TravelLevelName;
	
	// 현재 GameState 참조(플레이어/게임 흐름 정보 접근)
	UPROPERTY()
	TObjectPtr<APTWGameState> PTWGameState;
private:
	// 현재 라운드/플레이어 데이터를 Subsystem으로 저장
	void SaveGameDataToSubsystem();

	// 로그인한 플레이어에게 Subsystem 저장 데이터를 적용
	void ApplyPlayerDataFromSubsystem(APlayerController* NewPlayer);

	// 타이머 틱 처리(종료 시 EndTimer 트리거)
	void UpdateTimer();
	
	int32 CurrentRound;
	
	// 내부 타이머 핸들(StartTimer/UpdateTimer에서 사용)
	FTimerHandle TimerHandle;
};
