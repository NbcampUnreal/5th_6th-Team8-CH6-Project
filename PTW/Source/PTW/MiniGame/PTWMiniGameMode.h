// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWMiniGameRule.h"
#include "Inventory/PTWItemDefinition.h"
#include "PTW/CoreFramework/Game/GameMode/PTWGameMode.h"
#include "System/Prop/PTWPropData.h"
#include "PTWMiniGameMode.generated.h"

class APTWPlayerController;
class UPTWBaseControllerComponent;
class UPTWItemInstance;
class IPTWPlayerRoundDataInterface;
class UPTWChaosEventManager;
class UGameplayEffect;
class AController;
class APTWPlayerState;
class UPTWItemDefinition;
class APTWWeaponActor;
class APTWResultCharacter;
class UPTWPropData;

struct FItemArrayWrapper
{
	TArray<TObjectPtr<UPTWItemInstance>> Items;
};

UCLASS()
class PTW_API APTWMiniGameMode : public APTWGameMode
{
	GENERATED_BODY()

public:
	APTWMiniGameMode();

	void AddWinPoint(AActor* Actor, int32 AddPoint);
	
	bool PlayerDeadCheck(AController* Controller);
	
	FItemArrayWrapper GetOldPlayerItems(AController* Controller) const;
	
	/** 플레이어 사망할 때 호출되는 함수 */
	UFUNCTION()
	virtual void HandlePlayerDeath(AActor* DeadActor, AActor* KillActor);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Rule")
	FPTWMiniGameRule MiniGameRule;
protected:
	virtual void InitGameState() override;
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Logout(AController* Exiting) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual void HandleSeamlessTravelPlayer(AController*& C) override;

	virtual void PlayerReadyToPlay(APlayerController* Controller) override;

	//* UI Component PlayerController에 연결 */
	virtual void AttachUIComponent(AController* Controller);
	
	//* 미니 게임 시작 */
	UFUNCTION()
	virtual void StartGame();
	virtual void StartRound();

	virtual void UpdateTimer() override;
	
	//* 타이머기 종료되면 호출되는 함수 */
	virtual void EndTimer() override;
	
	//* 라운드가 종료됐을 때 호출하는 함수 */
	virtual void EndRound();
	
	//* 미니 게임이 완전히 끝났을 때 호출하는 함수 */
	virtual void EndGame();
	
	/** 미니 게임 룰에 따라 킬/데스,승점을 부여한다. */
	void AddKillDeathCount(APlayerState* DeadPlayerState, APlayerState* KillPlayerState);
	/** 설정된 점수 부여. */
	void AddRoundScore(APlayerState* ScoreTarget, int32 ScoreValue = 1);
	
	virtual void SpawnDefaultWeapon(AController* NewPlayer);

	//* 미니 게임 라운드 시작 대기 */
	virtual void WaitingToStartRound();
	
	/** 카운트 다운 시작 */
	void StartCountDown();
	/** 매초마다 카운트다운 감소 */
	void TickCountDown();
	
	/** 카운트 다운 종료 시 호출 */
	UFUNCTION()
	virtual void OnCountDownFinished();
	
	void ApplyRoundPropRandom();

	/** 플레이어 모든 태그 제거*/
	void RemoveTags(AController* Controller);
	
	/** 승리 조건 체크 */
	virtual void CheckEndGameCondition();
	virtual void CheckSurvivalCondition();
	virtual void CheckTargetCondition();

	/** 팀 결정 */
	virtual void AssignTeam();
	
	/* 코인 스폰 타이머용 함수 */ 
	void OnCoinSpawnTimerElapsed();
	
	//연출 단계 함수
	void StartResultSequence();
	void FinishEndGameSequence();

	// FIXME : 임시로 관전상태 해제테스트
	/* 플레이어 관전상태 해제 */
	void ExitSpectatorMode(AController* Controller);

	/* 인벤토리 아이템에 따른 아이템 지급 함수 */
	void SpawnPlayerSavedItems(AController* Controller);

	void StartChaosEvent();

protected:
	
	UPROPERTY(EditDefaultsOnly, Category="Prop")
	bool bApplyPropOnStartGame = false;

	UPROPERTY(EditDefaultsOnly, Category="Prop", meta=(EditCondition="!bApplyPropOnStartGame"))
	bool bApplyPropOnStartRound = true; 

	UPROPERTY(EditDefaultsOnly, Category="Prop", meta=(EditCondition="bApplyPropOnStartGame || bApplyPropOnStartRound"))
	TObjectPtr<UPTWPropData> RoundPropData;
	
	UPROPERTY(EditDefaultsOnly, Category = "Game|Weapon")
	TObjectPtr<UPTWItemDefinition> ItemDefinition;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPTWBaseControllerComponent> ControllerComponentClass;
	
	//UPROPERTY()
	//TArray<TObjectPtr<APlayerStart>> PlayerStarts;

	UPROPERTY(EditDefaultsOnly, Category = "Rule|Result")
	TSubclassOf<class APTWResultCharacter> ResultCharacterClass;
	UPROPERTY(EditDefaultsOnly, Category = "Rule|Result")
	float ResultSequenceDuration = 15.0f;

	
	/* 이미 게임 종료가 호출됬는지 체크 */
	bool bIsGameEnded = false;
	
	FTimerHandle CountDownTimerHandle;
	FTimerHandle CoinSpawnTimerHandle;
	FTimerHandle ResultTimerHandle;

private:
	/** 플레이어에게 미니 게임 태그 적용 */
	void ApplyMiniGameTag(AController* NewPlayer);
	
	/** 마지막으로 사망한 플레이어 찾기 */
	IPTWPlayerRoundDataInterface* FindLastDeadPlayer();
	
	/** 라운드 종료 시 플레이어의 라운드 전용 데이터 초기화*/
	void ResetPlayerRoundData();

	/**라운드 종료 시 플레이어의 인벤토리 ID 초기화 */
	void ResetPlayerInventoryID();

	/** 플레이어 리스폰 */
	void RespawnPlayer(APTWPlayerController* SpawnPlayerController);
	
	/** 리스폰 시 플레이어 체력 초기화 */
	void InitPlayerHealth(AController* Controller);
	
	/* 플레이어 인벤토리 ItemInstance Outer 재설정*/
	void SetOldPlayerItemInstanceOuter(TArray<TObjectPtr<UPTWItemInstance>> ItemArr);

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> MiniGameEffectClass;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPTWChaosEventManager> ChaosEventManager;

	/* 코인 생성 주기 (초) */
	UPROPERTY(EditDefaultsOnly)
	float CoinSpawnInterval = 3.0f;
	
	int32 PlayerStartCount = 0;

	int32 CurrentDeathOrder = 1;
	
	TMap<AController*, FItemArrayWrapper> PendingRespawnItems;
};
