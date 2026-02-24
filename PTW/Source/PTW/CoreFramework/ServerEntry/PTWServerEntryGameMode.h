// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "PTWServerEntryGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API APTWServerEntryGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	APTWServerEntryGameMode();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
