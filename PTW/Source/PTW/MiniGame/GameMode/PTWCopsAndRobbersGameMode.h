// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MiniGame/PTWMiniGameMode.h"
#include "PTWCopsAndRobbersGameMode.generated.h"

#define ROBBERS 0
#define COPS 1

class UGameplayAbility;

UCLASS()
class PTW_API APTWCopsAndRobbersGameMode : public APTWMiniGameMode
{
	GENERATED_BODY()
	
public:
	APTWCopsAndRobbersGameMode();
	
	virtual void StartGame() override;
	virtual void WaitingToStartRound() override;
	
protected:
	UPROPERTY(EditAnywhere, Category="GAS")
	TSubclassOf<UGameplayAbility> GA_Blind;
};
