// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTW/CoreFramework/Game/GameMode/PTWGameMode.h"
#include "PTWLobbyGameMode.generated.h"

/**
 * 게임 라운드 및 진행 흐름에 대한 규칙 정의 구조체
 * - 로비 대기, 라운드 수, 플레이어 제한 등
 * - GameMode에서 게임 시작/진행 조건 판단에 사용
 */
USTRUCT(BlueprintType)
struct FPTWGameFlowRule
{
	GENERATED_BODY()

	/** 로비에서 최소 인원이 충족 됐을 때 플레이어 대기 시간(초) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GameFlow")
	float WaitingTime = 60.f;

	/** 미니 게임 종료 후 다음 미니 게임 시작까지의 대기 시간(초) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GameFlow")
	float NextMiniGameWaitTime = 60.f;

	/** 한 게임에서 진행 가능한 최대 라운드 수 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GameFlow")
	int32 MaxRound = 10;

	/** 게임을 시작하기 위한 최소 플레이어 수 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GameFlow")
	int32 MinPlayersToStart = 2;

	/** 게임에 동시에 참여할 수 있는 최대 플레이어 수 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GameFlow")
	int32 MaxPlayer = 16;
};

/**
 * 
 */
UCLASS()
class PTW_API APTWLobbyGameMode : public APTWGameMode
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GameFlow")
	FPTWGameFlowRule GameFlowRule;
	
protected:
	virtual void InitGameState() override;
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
private:
	void AddRandomGold(APlayerController* NewPlayer);
	
	/**  */
	bool bIsFirstLobby;
	bool bWaitingTimerStarted = false;
	
};
