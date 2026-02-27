// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "CoreFramework/Game/GameInstance/PTWGameInstance.h"
#include "CoreFramework/Game/GameState/PTWGameData.h"
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

	/** 타이머 종료 시 호출(타이머 정리 및 종료 후 처리 트리거) */
	UFUNCTION()
	virtual void EndTimer();
protected:
	/** 게임 월드 시작 시 초기 설정 및 GameState 참조 캐싱 */
	virtual void InitGameState() override;
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void BeginPlay() override;
	
	/** 플레이어 로그인 시 호출(접속 인원/시작 조건 갱신) */
	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	
	/** 플레이어 로그아웃 시 호출(접속 인원/게임 진행 상태 갱신) */
	virtual void Logout(AController* Exiting) override;

	virtual void GetSeamlessTravelActorList(bool bToTransition, TArray<AActor*>& ActorList) override;;

	virtual void PostSeamlessTravel() override;

	/* 트래블 직전에 모든 클라에 로딩 정보 전달 */
	void PrepareAllPlayersLoadingScreen(ELoadingScreenType Type, FName MapRowName);

public:
	/** 모든 플레이어가 로딩되었는지 확인 */
	void CheckAllPlayersLoaded();

	/** 준비 완료된 플레이어 처리 **/
	virtual void PlayerReadyToPlay(AController* Controller);
	
	/** 모든 클라이언트의 로딩 화면을 끄라는 RPC */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_CloseLoadingScreen();

	
protected:
	/** 지정한 시간(초) 기준으로 타이머를 시작 */
	void StartTimer(float TimeDuration);

	/** 기본 게임 타이머 초기화 */
	void ClearTimer();
	
	/** 설정된 TravelLevelName으로 레벨 이동 처리 */
	void TravelLevel();

	/** 플레이어를 시작 위치로 이동시키는 함수 */
	void MovePlayerToStart(AController* Controller);

	// 타이머 틱 처리(종료 시 EndTimer 트리거)
	virtual void UpdateTimer();
	
	void SetInputBlock(bool bInputBlock);
	// 이동할 레벨 이름(TravelLevel에서 사용)
	FString TravelLevelName;
	
	// 현재 GameState 참조(플레이어/게임 흐름 정보 접근)
	UPROPERTY()
	TObjectPtr<APTWGameState> PTWGameState;

	// 내부 타이머 핸들(StartTimer/UpdateTimer에서 사용)
	FTimerHandle TimerHandle;

	bool bAllPlayerReady = false;
	int32 ReadyPlayer = 0;
	int32 AllPlayer = 0;

	FPTWGameData CachedGameData;
private:
	// 현재 라운드/플레이어 데이터를 Subsystem으로 저장
	void SaveGameDataToSubsystem();

	// 로그인한 플레이어에게 Subsystem 저장 데이터를 적용
	void ApplyPlayerDataFromSubsystem(APlayerController* NewPlayer);

	
	
	int32 CurrentRound;

	
};
