// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CoreFramework/PTWPlayerData.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PTWScoreSubsystem.generated.h"


/**
 * 게임 전체에서 사용되는 점수 및 라운드 정보를 관리하는 Subsystem
 * - 플레이어 누적 데이터 유지
 * - 게임 라운드 진행 상태 관리
 */
UCLASS()
class PTW_API UPTWScoreSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** 특정 플레이어의 데이터를 저장
	 * - PlayerIndex를 키로 플레이어 진행 데이터 갱신
	 *
	 * @param PlayerName 플레이어를 식별하는 인덱스
	 * @param PlayerData 저장할 플레이어 데이터
	 */
	void SavePlayerData(const FString& PlayerName, const FPTWPlayerData& PlayerData);

	/** 현재 게임 라운드 값을 저장
	 * - 라운드 진행 시 호출되어 상태 갱신
	 *
	 * @param NewGameRound 저장 할 현재 게임 라운드 번호
	 */
	void SaveGameRound(int32 NewGameRound);


	bool bIsFirstLobby = true;
	
	/** 저장된 라운드 반환 */
	FORCEINLINE int32 GetCurrentGameRound() const { return SavedGameRound; }
	
	/** 지정한 플레이어의 저장된 데이터가 있으면 반환 */
	FPTWPlayerData* FindPlayerData(const FString& PlayerName);

private:
	// 현재 저장 게임 라운드 번호
	int32 SavedGameRound = 1;
	
	// 플레이어 ID를 키로 하는 플레이어 데이터 저장소
	TMap<FString, FPTWPlayerData> SavedPlayersData;
};
