// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MiniGame/PTWMiniGameMode.h"
#include "PTWDeliveryGameMode.generated.h"

class UAbilitySystemComponent;
class APTWPlayerCharacter;
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
	
protected:
	void GiveDeliveryItems(APTWPlayerCharacter* TargetCharacter);
	
private:
	/* 미니 게임 시작 이펙트 적용 */
	void ApplyMiniGameEffect(APTWPlayerCharacter* TargetCharacter);
	
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
	TSubclassOf<UGameplayEffect> DeliveryStartEffect;
};
