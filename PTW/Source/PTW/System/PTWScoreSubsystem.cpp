// Fill out your copyright notice in the Description page of Project Settings.


#include "System/PTWScoreSubsystem.h"

#include "CoreFramework/PTWPlayerState.h"


void UPTWScoreSubsystem::SavePlayerData(const FString& PlayerName, const FPTWPlayerData& PlayerData)
{
	SavedPlayersData.Add(PlayerName, PlayerData);
}

void UPTWScoreSubsystem::SaveLobbyItemData(const FString& PlayerName, const FPTWLobbyItemData& LobbyItemData)
{
	SavedLobbyItemData.Add(PlayerName, LobbyItemData);
}

void UPTWScoreSubsystem::SaveGameRound(int32 NewGameRound)
{
	SavedGameRound = NewGameRound;
}

void UPTWScoreSubsystem::SaveAllPlayerCount(int32 NewPlayerCount)
{
	SavedAllPlayerCount = NewPlayerCount;
}

void UPTWScoreSubsystem::SaveGameData(const FPTWGameData& GameData)
{
	SavedGameData = GameData;
}

void UPTWScoreSubsystem::IncreasePlayerCount()
{
	++SavedAllPlayerCount;
}

void UPTWScoreSubsystem::DecreasePlayerCount()
{
	--SavedAllPlayerCount;
}

FPTWPlayerData* UPTWScoreSubsystem::FindPlayerData(const FString& PlayerName)
{
	return SavedPlayersData.Find(PlayerName);
}

FPTWLobbyItemData* UPTWScoreSubsystem::FindLobbyItemData(const FString& PlayerName)
{
	return SavedLobbyItemData.Find(PlayerName);
}






