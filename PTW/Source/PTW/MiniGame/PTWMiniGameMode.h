// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTW/CoreFramework/Game/GameMode/PTWGameMode.h"
#include "PTWMiniGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API APTWMiniGameMode : public APTWGameMode
{
	GENERATED_BODY()


protected:
	virtual void BeginPlay() override;


	/** 미니게임 진행 시간 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Game|Timer")
	float MiniGameTime = 90;
};
