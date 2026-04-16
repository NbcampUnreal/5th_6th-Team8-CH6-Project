// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CoreFramework/PTWPlayerData.h"
#include "CoreFramework/Game/GameState/PTWGameData.h"
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
	void SavePlayerGameData(const FString& PlayerID, const FPTWPlayerGameData& PlayerGameData);
	void SaveServerTravelPlayerCount(int32 NewPlayerCount);
	void SaveGameData(const FPTWGameData& GameData);

	UFUNCTION()
	void AddConnectedPlayerId(const FString& ConnectedPlayerId);

	void AddTravelPlayerId(const FString& TravelPlayerId, const FString& PlayerName);
	
	/** 지정한 플레이어의 저장된 데이터가 있으면 반환 */
	FPTWPlayerGameData* FindPlayerGameData(const FString& PlayerId);

	void RemoveTravelPlayersId();
	
	FORCEINLINE FPTWGameData GetSavedGameData() const { return SavedGameData; }
	FORCEINLINE TMap<FString, FPTWPlayerGameData> GetConnectedPlayersGameData() const {return ConnectedPlayersGameData;}
	FORCEINLINE TMap<FString, FString> GetTravelPlayersId() const { return TravelPlayersId; }
	FORCEINLINE int32 GetServerTravelPlayerCount() const { return ServerTravelPlayerCount; }
protected:
	virtual void BeginPlay();

private:
	int32 ServerTravelPlayerCount = 0;
	
	UPROPERTY()
	FPTWGameData SavedGameData;

	//* 세션에 참가한 이력이 있는 플레이어의 게임 데이터 */ 
	TMap<FString, FPTWPlayerGameData> ConnectedPlayersGameData;

	//* 다음 레벨로 이동 한 플레이어 데이터 */ 
	TMap<FString, FString> TravelPlayersId;
public:
	
	bool bIsFirstLobby = true;
	
	UPROPERTY()
	TArray<FString> TravelingBotNames;
};
