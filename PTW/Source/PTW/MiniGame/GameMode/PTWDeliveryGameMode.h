// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MiniGame/PTWMiniGameMode.h"
#include "PTWDeliveryGameMode.generated.h"

class AStartBlockActor;
class ARaceTrack;
class UAbilitySystemComponent;
class APTWPlayerCharacter;
class IPTWCombatInterface;
class UPTWDeliveryControllerComponent;
class APlayerStart;

/**
 * 
 */
UCLASS()
class PTW_API APTWDeliveryGameMode : public APTWMiniGameMode
{
	GENERATED_BODY()
	
public:
	APTWDeliveryGameMode();
	
	virtual void StartRound() override;
	
	/* 초반 배달 아이템 및 GE 적용 */
	void GiveDeliveryItems(APTWPlayerCharacter* TargetCharacter, TSubclassOf<UGameplayEffect> EffectToApply);
	
	/* 도착 지점에 도착했을 때 배열에 저장시키는 함수 */
	void GoalPlayer(APTWPlayerCharacter* TargetCharacter, TSubclassOf<UGameplayEffect> EffectToApply);
	
	/* 충전 시작*/
	void StartBatteryCharge(APTWPlayerCharacter* TargetCharacter);
	
	/* 충전 완료*/
	void EndBatteryCharge(APTWPlayerCharacter* TargetCharacter);
	
	/* 세이브 포인트로 해당 플레이어 스폰*/
	void SetPlayerSpawnLocation(APTWPlayerController* PC, FVector NewLocation);
	
	/* 세이브 포인트 위치 반환*/
	FTransform GetPlayerSpawnTransform(APTWPlayerController* PC);

	/* GE 적용 */
	void ApplyGameEffect(APTWPlayerCharacter* Target, TSubclassOf<UGameplayEffect> TargetGameplayEffect);
	
	FORCEINLINE APTWPlayerController* GetLeaderController() const { return RankPCList[0];}
	
	UFUNCTION(BlueprintCallable, Category = "Delivery | Logic")
	FRandomItemBoxData GetRandomItemRowFromTable();
	
	void InitializeRaceRankingUI();
	
protected:
	virtual void HandlePlayerDeath(AActor* DeadActor, AActor* KillActor) override;
	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual void BeginPlay() override;
	virtual void StartCountDown() override;
	virtual AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName) override;
	
	/* 카운트 다운 위젯 표시를 위한 함수 */
	void StartEndCountDown();
	/* 카운트 업데이트 */
	void UpdateCountDown();
	
	void StopCountDown();
	
	/* 플레이어 사망 체크 */
	bool CheckingDeadPlayer(AController* NewPlayer);
	
	/* 점수 부여 로직 */
	void GiveRoundScore();
	
	/* CombatInterface 캐스팅 공통 함수 */
	IPTWCombatInterface* CastToPTWCombatInterface(APTWPlayerCharacter* PlayerCharacter);
	
	/* 거리 계산 로직 */
	float GetDistanceForActor(AActor* TargetActor);
	
	/* 등수 계산 로직 */
	void UpdateAllPlayerRanks();
	
	/* 초반 알림 UI */
	void SendMessgeBeginPlay();
	
	/* 초반 GE 적용 */
	void ApplyBeginPlayEffect(APTWPlayerController* PC);
	
	/* 랜덤 아이템 어빌리티 미리 부여 */
	void GrantItemAbilities(UAbilitySystemComponent* ASC);

private:
	/* 미니 게임 시작 무기 지급*/
	void GivingDefaultWeapon(APTWPlayerCharacter* TargetCharacter);
	
	/* 미니 게임 룰 적용*/
	void SetMiniGameRule();
	
	/* 플레이어에게 미니게임 전용 AS 부여*/
	void GrantDeliveryAttributeSet();
	
	/* AS 할당 이후 Value값 초기화 */
	void InitializeAttributeSet(UAbilitySystemComponent* TargetASC);
	
	/* UI */
	void DeliveryUISetting(APTWPlayerCharacter* TargetCharacter);
	
	/* 결과 연출에서 사용하는 승리자 체크 로직 */
	virtual bool IsWinner(APTWPlayerState* PlayerState) override;
protected:
	UPROPERTY(EditAnywhere, Category = "GAS|Effect")
	TSubclassOf<UGameplayEffect> KillBonusEffect;
	
	UPROPERTY(EditAnywhere, Category = "GAS|Effect")
	TSubclassOf<UGameplayEffect> RestartPlayerEffect;
	
	/* 등수 표시를 위한 도착 지점에 도착한 플레이어 배열*/
	UPROPERTY(VisibleAnywhere, Category = "Game|Winner")
	TArray<APTWPlayerCharacter*> GoalPlayers;
	
	/* 무기와 배달물을 지급 받은 플레이어 Set */
	UPROPERTY(VisibleAnywhere, Category = "Game|Default")
	TArray<AController*> DeliveredCharacters;
	
	UPROPERTY(EditAnywhere, Category = "Game|Weapon")
	TObjectPtr<UPTWItemDefinition> DeliveryDefaultWeapon;
	
	UPROPERTY()
	TObjectPtr<APlayerStart> SharedCheckPointStart;
	
	UPROPERTY(EditAnywhere)
	TArray<APlayerStart*> PlayerStartPoints; 
	
	UPROPERTY(EditAnywhere, Category = "Game|Ranking")
	TObjectPtr<ARaceTrack> RaceTrackSpline; 
	
	UPROPERTY(EditAnywhere, Category = "GAS|Effect")
	TSubclassOf<UGameplayEffect> BeginApplyEffect;
	
	UPROPERTY(EditAnywhere, Category = "Actors")
	TObjectPtr<AStartBlockActor> StartBlocker;
	
	UPROPERTY(EditAnywhere, Category = "Delivery | Data")
	TObjectPtr<UDataTable> ItemDataTable;

private:
	FTimerHandle CountDownTimerHandle;
	FTimerHandle RankingTimerHandle;
	
	int32 FinalCount = 10;
	
	TMap<APTWPlayerController*, FVector> PlayerSpawnPoints; 
	
	TArray<APTWPlayerController*> RankPCList;
	
	bool bIsCountDownEnded = false;
};
