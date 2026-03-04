// Fill out your copyright notice in the Description page of Project Settings.


#include "CoreFramework/PTWGameUserSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Culture.h"

void UPTWGameUserSettings::ApplyAudioSettings(UWorld* World, USoundMix* SoundMix, USoundClass* MasterClass, USoundClass* BGMClass, USoundClass* SFXClass, USoundClass* UIClass)
{
	if (!World || !SoundMix) return;

	const float FinalMaster = MasterVolume;
	const float FinalBGM = MasterVolume * BGMVolume;
	const float FinalSFX = MasterVolume * SFXVolume;
	const float FinalUI = MasterVolume * UIVolume;

	if (MasterClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(
			World, SoundMix, MasterClass, FinalMaster, 1.0f, 0.0f, true);
	}

	if (BGMClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(
			World, SoundMix, BGMClass, FinalBGM, 1.0f, 0.0f, true);
	}

	if (SFXClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(
			World, SoundMix, SFXClass, FinalSFX, 1.0f, 0.0f, true);
	}

	if (UIClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(
			World, SoundMix, UIClass, FinalUI, 1.0f, 0.0f, true);
	}

	UGameplayStatics::PushSoundMixModifier(World, SoundMix);
}

void UPTWGameUserSettings::ApplyLanguageSettings()
{
	if (!SelectedLanguage.IsEmpty())
	{
		FInternationalization::Get().SetCurrentCulture(SelectedLanguage);
	}
}
