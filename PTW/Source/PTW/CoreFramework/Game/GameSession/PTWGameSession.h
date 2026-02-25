// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameSession.h"
#include "PTWGameSession.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API APTWGameSession : public AGameSession
{
	GENERATED_BODY()
	
protected:
	virtual void RegisterServer() override;
};
