// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#if WITH_GAMELIFT
#include "GameLiftServerSDK.h"
#endif
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
#if WITH_GAMELIFT
	void InitGameLift();
	TSharedPtr<FProcessParameters> ProcessParameters;
#endif
};
