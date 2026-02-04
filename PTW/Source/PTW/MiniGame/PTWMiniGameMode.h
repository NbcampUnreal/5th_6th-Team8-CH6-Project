// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWMiniGameRule.h"
#include "Inventory/PTWItemDefinition.h"
#include "PTW/CoreFramework/Game/GameMode/PTWGameMode.h"
#include "PTWMiniGameMode.generated.h"

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

	void AddWinPoint(APawn* Pawn, int32 Score);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Rule")
	FPTWMiniGameRule MiniGameRule;
protected:
	virtual void InitGameState() override;
	virtual void BeginPlay() override;
	virtual void EndTimer() override;

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	
	virtual void RestartPlayer(AController* NewPlayer) override;
	
	void SpawnDefaultWeapon(AController* NewPlayer);

	
	/** 미니게임 진행 시간 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Game|Timer")
	float MiniGameTime = 90;

	UPROPERTY(EditDefaultsOnly, Category = "Game|Weapon")
	TObjectPtr<UPTWItemDefinition> ItemDefinition;

	//UPROPERTY()
	//TArray<TObjectPtr<APlayerStart>> PlayerStarts;
	
	/** 라운드 시작 전 카운트 다운 10초 */
	UPROPERTY(EditDefaultsOnly, Category="Game|Timer")
	int32 StartCountDownTime = 10;
	/** 본게임 라운드 진행 시간 */
	UPROPERTY(EditDefaultsOnly, Category = "Game|Timer")
	float RoundPlayTime = 30.f;
	
	FTimerHandle CountDownTimerHandle;
	int32 CurrentCountDown = 0;
	/** 카운트 다운 시작 */
	void StartCountDown();
	/** 매초마다 카운트다운 감소 */
	void TickCountDown();
	
	virtual void OnCountDownFinished();
	
	

private:
	/** 플레이어 사망 처리 */
	UFUNCTION()
	void HandlePlayerDeath(AActor* DeadActor, AActor* KillActor);

	/** 미니 게임 룰에 따라 킬/데스,승점을 부여한다. */
	void UpdatePlayerRoundData(APlayerState* DeadPlayerState, APlayerState* KillPlayerState);
	
	/** 라운드 시작 시 플레이어의 라운드 전용 데이터 초기화*/
	void ResetPlayerRoundData();

	void RespawnPlayer(APTWPlayerController* SpawnPlayerController);
	
	/** 리스폰 시 플레이어 체력 초기화 */
	void InitPlayerHealth(AController* Controller);

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> MiniGameEffectClass;
	
	int32 PlayerStartCount = 0;

};
