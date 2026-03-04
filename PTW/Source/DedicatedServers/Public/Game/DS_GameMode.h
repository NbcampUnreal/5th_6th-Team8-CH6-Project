// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#if WITH_GAMELIFT
#include "GameLiftServerSDK.h"
#endif
#include "DS_GameMode.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(GameServerLog, Log, All);

UCLASS()
class DEDICATEDSERVERS_API ADS_GameMode : public AGameMode
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	
private:
#if WITH_GAMELIFT
	void InitGameLift();
	TSharedPtr<FProcessParameters> ProcessParameters;
#endif
};

