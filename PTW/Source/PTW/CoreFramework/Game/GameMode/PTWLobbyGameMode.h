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
	//virtual void PostLogin(APlayerController* NewPlayer) override;

private:
	void StartMiniGame();
	
	/**
 * 게임을 시작하기 위한 최소 플레이어 수
 * - 이 인원 이상 접속 시 대기 타이머 시작
 */
	UPROPERTY(EditDefaultsOnly, Category = "Game || Start")
	int32 MinPlayersToStart = 4;

	/**
	* 게임에 동시에 참여할 수 있는 최대 플레이어 수
	* - 서버에 접속 가능한 최대 인원 제한
	* - 이 값을 초과하면 추가 플레이어 입장 불가
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Game || Start")
	int32 MaxPlayer = 16;
	
	/**
	 * 최소 인원 충족 후 게임 시작까지의 대기 시간 (초)
	 * - 플레이어 추가 접속을 기다리기 위한 유예 시간
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Game || Start")
	float WaitingTime = 60.f;
};
