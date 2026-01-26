// Fill out your copyright notice in the Description page of Project Settings.


#include "System/PTWScoreSubsystem.h"

#include "CoreFramework/PTWPlayerState.h"


void UPTWScoreSubsystem::SavePlayerData(const FString& PlayerName, const FPTWPlayerData& PlayerData)
{
	SavedPlayersData.Add(PlayerName, PlayerData);
}

void UPTWScoreSubsystem::SaveGameRound(int32 NewGameRound)
{
	SavedGameRound = NewGameRound;
}

FPTWPlayerData* UPTWScoreSubsystem::FindPlayerData(const FString& PlayerName)
{
	return SavedPlayersData.Find(PlayerName);
}




