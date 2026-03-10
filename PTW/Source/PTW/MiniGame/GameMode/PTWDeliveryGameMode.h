// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MiniGame/PTWMiniGameMode.h"
#include "PTWDeliveryGameMode.generated.h"

class UAbilitySystemComponent;
class APTWPlayerCharacter;
class IPTWCombatInterface;
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
	
	void GiveDeliveryItems(APTWPlayerCharacter* TargetCharacter, TSubclassOf<UGameplayEffect> EffectToApply);
	
	/* 도착 지점에 도착했을 때 배열에 저장시키는 함수 */
	void GoalPlayer(APTWPlayerCharacter* TargetCharacter, TSubclassOf<UGameplayEffect> EffectToApply);
	
	/* 충전 시작*/
	void StartBatteryCharge(APTWPlayerCharacter* TargetCharacter);
	
	/* 충전 완료*/
	void EndBatteryCharge(APTWPlayerCharacter* TargetCharacter);

protected:
	virtual void HandlePlayerDeath(AActor* DeadActor, AActor* KillActor) override;
	void ApplyGameEffect(APTWPlayerCharacter* Target, TSubclassOf<UGameplayEffect> TargetGameplayEffect);
	virtual void OnCountDownFinished() override;
private:
	
	/* 미니 게임 시작 무기 지급*/
	void GivingDefaultWeapon(APTWPlayerCharacter* TargetCharacter);
	
	/* 미니 게임 룰 적용*/
	void SetMiniGameRule();
	
	/* 플레이어에게 미니게임 전용 AS 부여*/
	void GrantDeliveryAttributeSet();
	
	/* AS 할당 이후 Value값 초기화 */
	void InitializeAttributeSet(UAbilitySystemComponent* TargetASC);

protected:
	UPROPERTY(EditAnywhere, Category = "GAS|Effect")
	TSubclassOf<UGameplayEffect> KillBonusEffect;
	
	/* 등수 표시를 위한 도착 지점에 도착한 플레이어 배열*/
	UPROPERTY(VisibleAnywhere, Category = "Game|Winner")
	TArray<APTWPlayerCharacter*> GoalPlayers;
	
	/* 무기와 배달물을 지급 받은 플레이어 Set */
	UPROPERTY(VisibleAnywhere, Category = "Game|Default")
	TSet<APTWPlayerCharacter*> DeliveredCharacters;
	
	UPROPERTY(EditAnywhere, Category = "Game|Weapon")
	TObjectPtr<UPTWItemDefinition> DeliveryDefaultWeapon;
};
