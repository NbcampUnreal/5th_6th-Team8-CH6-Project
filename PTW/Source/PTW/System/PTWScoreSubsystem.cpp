// Fill out your copyright notice in the Description page of Project Settings.


#include "System/PTWScoreSubsystem.h"

#include "CoreFramework/PTWPlayerState.h"



void UPTWScoreSubsystem::SavePlayerData(int32 PlayerIndex, const FPTWPlayerData& PlayerData)
{
	SavedPlayersData.Add(PlayerIndex, PlayerData);
}

void UPTWScoreSubsystem::SaveGameRound(int32 NewGameRound)
{
	SavedGameRound = NewGameRound;
}

FPTWPlayerData* UPTWScoreSubsystem::FindPlayerData(int32 PlayerID)
{
	return SavedPlayersData.Find(PlayerID);
}


