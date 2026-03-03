// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MiniGame/PTWMiniGameMode.h"
#include "PTWDeliveryGameMode.generated.h"

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

protected:
	UPROPERTY()
	TSubclassOf<UGameplayEffect> DeliveryStartEffect;
	
	UPROPERTY()
	TObjectPtr<UPTWItemDefinition> DefaultWeaponDef;
};
