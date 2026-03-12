// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CoreFramework/PTWPlayerController.h"
#include "PTWAbyssPlayerController.generated.h"

class APostProcessVolume;

UCLASS()
class PTW_API APTWAbyssPlayerController : public APTWPlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(Client, Reliable)
	void Client_SetAbyssDark(bool bEnable);

protected:
	UPROPERTY()
	TObjectPtr<APostProcessVolume> CachedAbyssPP = nullptr;

	void CacheAbyssPP();
};
