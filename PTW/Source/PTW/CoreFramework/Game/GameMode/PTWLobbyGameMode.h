// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PTW/CoreFramework/Game/GameMode/PTWGameMode.h"
#include "PTWLobbyGameMode.generated.h"

class APTWPlayerState;
class UPTWLobbyItemManager;
class UPTWRoundEventManager;
struct FPTWMiniGameMapRow;
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
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GameFlow")
	float RouletteDelay = 5.f;
	
	/** 미니 게임 종료 후 다음 미니 게임 시작까지의 대기 시간(초) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GameFlow")
	float NextMiniGameWaitTime = 60.f;

	/** 한 게임에서 진행 가능한 최대 라운드 수 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GameFlow")
	int32 MaxRound = 10;

	/** 게임을 시작하기 위한 최소 플레이어 수 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GameFlow")
	int32 MinPlayersToStart = 4;
	
	/** 최소 인원이 충족되면 자동으로 게임을 시작 할 지 여부 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GameFlow")
	bool bAutoStartWhenMinPlayersMet = true;

	/** 최대 인원 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GameFlow")
	int32 MaxPlayers = 16;
	
};

/**
 * 
 */
UCLASS()
class PTW_API APTWLobbyGameMode : public APTWGameMode
{
	GENERATED_BODY()

public:
	APTWLobbyGameMode();

	void ApplyLobbyItem(APTWPlayerState* Buyer, const FName ItemId, APTWPlayerState* WinTarget = nullptr);
	void AddChaosItemEntry(const FPTWChaosItemEntry& Entry);
	void AddGold(APTWPlayerState* PlayerState, int32 Amount);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GameFlow")
	FPTWGameFlowRule GameFlowRule;
	
	/* 로비 복귀시에 지급될 골드 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GameFlow")
	int32 RoundClearBonusGold = 300;

	UFUNCTION()
	void OnRouletteFinished(FName SelectedMapName);

protected:
	virtual void InitGameState() override;
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void HandleSeamlessTravelPlayer(AController*& C) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void PlayerReadyToPlay(APlayerController* Controller) override;
	
	//* PreGameLobby 상태에서 타이머가 종료되면 게임 시작 /
	void StartGameLobby();

	virtual void EndTimer() override;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPTWRoundEventManager> RoundEventManager;

	/** 로비 아이템 관리 및 적용 */
	UPROPERTY()
	TObjectPtr<UPTWLobbyItemManager> LobbyItemManager;
private:
	void ExitSpectorMode(AController* Controller);
	
	void StartRoulette();
	void EndGame();
	void ReturnToMainMenu();
	
	/** 로비 아이템 데이터 테이블 */
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UDataTable> LobbyItemDataTable;
	
	UPROPERTY(EditDefaultsOnly, Category = "Test")
	bool bSkipFirstLobby = false;
	
	bool bIsFirstLobby;
	bool bWaitingTimerStarted = false;

	FTimerHandle LoadingDelayTimer;
	FTimerHandle TestTimer;
};
