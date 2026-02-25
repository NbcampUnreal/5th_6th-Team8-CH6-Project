// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GameLiftServerSDK.h"
#include "PTWServerEntryGameMode.generated.h"
/**
 * 
 */

DECLARE_LOG_CATEGORY_EXTERN(GameServerLog, Log, All);

UCLASS()
class PTW_API APTWServerEntryGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	APTWServerEntryGameMode();
	
protected:
	virtual void BeginPlay() override;
	
private:
	TSharedPtr<FProcessParameters> ProcessParameters;
	
	void InitGameLift();
};
