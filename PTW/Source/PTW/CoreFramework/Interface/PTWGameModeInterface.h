// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PTWGameModeInterface.generated.h"

class APTWPlayerState;
// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPTWGameModeInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PTW_API IPTWGameModeInterface
{
	GENERATED_BODY()


public:
	virtual void EndGame() = 0;
	virtual void RestartPlayer(AController*) = 0;
	virtual void TravelLevel() = 0;
	virtual bool IsWinner(APTWPlayerState* PlayerState) = 0;
};
