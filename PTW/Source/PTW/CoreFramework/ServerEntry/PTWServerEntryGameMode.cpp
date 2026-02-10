// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWServerEntryGameMode.h"
#include "BlueprintDataDefinitions.h"
#include "Kismet/GameplayStatics.h"
#include "System/PTWSessionSubsystem.h"

void APTWServerEntryGameMode::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Display, TEXT("PTWMainMenuGameMode BeginPlay"));
	
	if (IsRunningDedicatedServer())
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UPTWSessionSubsystem* SessionSubsystem = GI->GetSubsystem<UPTWSessionSubsystem>())
			{
				int32 MaxPlayers = UGameplayStatics::GetIntOption(OptionsString, TEXT("MaxPlayers"), 10);
				bool bIsPrivate = UGameplayStatics::GetIntOption(OptionsString, TEXT("bIsPrivate"), 0) > 0;
				
				TArray<FSessionPropertyKeyPair> ReconstructedSettings;
				
				// FString MapSettingVal = UGameplayStatics::ParseOption(OptionsString, TEXT("LobbyName"));
				// if (!MapSettingVal.IsEmpty())
				// {
				// 	FSessionPropertyKeyPair NewSetting;
				// 	NewSetting.Key = FName("LobbyName");
				// 	NewSetting.Data = MapSettingVal;
				// 	ReconstructedSettings.Add(NewSetting);
				// }
				
				// SessionSubsystem->CreateGameSession(ReconstructedSettings, MaxPlayers, bIsPrivate);
			}
		}
	}
}




