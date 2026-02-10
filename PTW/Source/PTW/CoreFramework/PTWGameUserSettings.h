// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "PTWGameUserSettings.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API UPTWGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()
	
public:

	UPROPERTY(config)
	float MasterVolume = 1.f;

	UPROPERTY(config)
	float MouseSensitivity = 1.f;
};
