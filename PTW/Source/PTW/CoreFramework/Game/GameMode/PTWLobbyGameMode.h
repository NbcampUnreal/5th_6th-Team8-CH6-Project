// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTW/CoreFramework/Game/GameMode/PTWGameMode.h"
#include "PTWLobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API APTWLobbyGameMode : public APTWGameMode
{
	GENERATED_BODY()

public:

protected:
	virtual void InitGameState() override;
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

private:
	void StartMiniGame();
	void AddRandomGold(APlayerController* NewPlayer);
	
	/**
	* 게임을 시작하기 위한 최소 플레이어 수
	* - 이 인원 이상 접속 시 대기 타이머 시작
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Game")
	int32 MinPlayersToStart = 4;

	/**
	* 게임에 동시에 참여할 수 있는 최대 플레이어 수
	* - 서버에 접속 가능한 최대 인원 제한
	* - 이 값을 초과하면 추가 플레이어 입장 불가
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Game")
	int32 MaxPlayer = 16;

	/** 로비에서 다음 게임 시작 전 대기 시간 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Game|Timer")
	float LobbyWaitingTime = 60;

	bool bIsFirstLobby;
};
