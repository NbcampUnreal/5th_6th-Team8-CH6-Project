// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MiniGame/PTWMiniGameMode.h"
#include "PTWAbilityBattleGameMode.generated.h"

class UPTWRandomDraftSystem;
/**
 * 
 */
UCLASS()
class PTW_API APTWAbilityBattleGameMode : public APTWMiniGameMode
{
	GENERATED_BODY()

protected:

	virtual void StartGame() override;

	void InitAttributeSet();
private:
	void GrandAbilityBattleAttributeSet();

	UPROPERTY()
	TObjectPtr<UPTWRandomDraftSystem> RandomDraftSystem;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> InitAttributeEffectClass;
	
};
