// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWMiniGameRule.h"
#include "Inventory/PTWItemDefinition.h"
#include "PTW/CoreFramework/Game/GameMode/PTWGameMode.h"
#include "PTWMiniGameMode.generated.h"

class UPTWChaosEventManager;
class UGameplayEffect;
class APTWPlayerController;
class APTWPlayerState;
class UPTWItemDefinition;
class APTWWeaponActor;

UCLASS()
class PTW_API APTWMiniGameMode : public APTWGameMode
{
	GENERATED_BODY()

public:
	APTWMiniGameMode();

	void AddWinPoint(AActor* Actor, int32 AddPoint);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Rule")
	FPTWMiniGameRule MiniGameRule;
protected:
	virtual void InitGameState() override;
	virtual void BeginPlay() override;
	virtual void Logout(AController* Exiting) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual void HandleSeamlessTravelPlayer(AController*& C) override;
	
	//* 미니 게임 시작 */
	UFUNCTION()
	virtual void StartGame();
	virtual void StartRound();
	
	//* 타이머기 종료되면 호출되는 함수 */
	virtual void EndTimer() override;
	
	//* 라운드가 종료됐을 때 호출하는 함수 */
	virtual void EndRound();
	
	//* 미니 게임이 완전히 끝났을 때 호출하는 함수 */
	virtual void EndGame();
	
	/** 플레이어 사망할 때 호출되는 함수 */
	UFUNCTION()
	void HandlePlayerDeath(AActor* DeadActor, AActor* KillActor);
	
	void SpawnDefaultWeapon(AController* NewPlayer);

	//* 미니 게임 라운드 시작 대기 */
	virtual void WaitingToStartRound();
	
	/** 카운트 다운 시작 */
	void StartCountDown();
	/** 매초마다 카운트다운 감소 */
	void TickCountDown();
	
	/** 카운트 다운 종료 시 호출 */
	UFUNCTION()
	virtual void OnCountDownFinished();
	
	/** 승리 조건 체크 */
	virtual void CheckEndGameCondition();

	/* 코인 스폰 타이머용 함수 */ 
	void OnCoinSpawnTimerElapsed();
	
	UPROPERTY(EditDefaultsOnly, Category = "Game|Weapon")
	TObjectPtr<UPTWItemDefinition> ItemDefinition;

	//UPROPERTY()
	//TArray<TObjectPtr<APlayerStart>> PlayerStarts;
	
	FTimerHandle CountDownTimerHandle;
	FTimerHandle CoinSpawnTimerHandle;

private:
	/** 미니 게임 룰에 따라 킬/데스,승점을 부여한다. */
	void UpdatePlayerRoundData(APlayerState* DeadPlayerState, APlayerState* KillPlayerState);

	/** 플레이어에게 미니 게임 태그 적용 */
	void ApplyMiniGameTag(AController* NewPlayer);
	
	/** 라운드 종료 시 플레이어의 라운드 전용 데이터 초기화*/
	void ResetPlayerRoundData();

	/**라운드 종료 시 플레이어의 인벤토리 ID 초기화 */
	void ResetPlayerInventoryID();

	/** 플레이어 리스폰 */
	void RespawnPlayer(APTWPlayerController* SpawnPlayerController);
	
	/** 리스폰 시 플레이어 체력 초기화 */
	void InitPlayerHealth(AController* Controller);

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> MiniGameEffectClass;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPTWChaosEventManager> ChaosEventManager;

	/* 코인 생성 주기 (초) */
	UPROPERTY(EditDefaultsOnly)
	float CoinSpawnInterval = 8.0f;
	
	int32 PlayerStartCount = 0;

	int32 CurrentDeathOrder = 1;
};
