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
	// 사용자가 설정한 오디오값 적용 (게임 재시작 같은 상황 시 Instance 에서 호출하여 저장값 적용)
	void ApplyAudioSettings(UWorld* World, USoundMix* SoundMix,
		USoundClass* MasterClass,
		USoundClass* BGMClass,
		USoundClass* SFXClass,
		USoundClass* UIClass);

	// 언어 적용 함수
	void ApplyLanguageSettings();

	UPROPERTY(config)
	float MasterVolume = 1.f;

	UPROPERTY(Config)
	float BGMVolume = 1.f;

	UPROPERTY(Config)
	float SFXVolume = 1.f;

	UPROPERTY(Config)
	float UIVolume = 1.f;

	UPROPERTY(config)
	float MouseSensitivity = 1.f;

	/* 언어 저장값 (디폴트: 영어) */
	UPROPERTY(Config)
	FString SelectedLanguage = TEXT("en");

	/* KeyGuide On/Off */
	bool bKeyGuideOn = true;
};
