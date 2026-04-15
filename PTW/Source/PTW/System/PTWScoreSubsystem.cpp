// Fill out your copyright notice in the Description page of Project Settings.


#include "System/PTWScoreSubsystem.h"

#include "CoreFramework/PTWPlayerState.h"


void UPTWScoreSubsystem::SavePlayerData(const FString& PlayerName, const FPTWPlayerData& PlayerData)
{
	if (!GetWorld() || !GetWorld()->GetAuthGameMode()) return;
	SavedPlayersData.Add(PlayerName, PlayerData);
}

void UPTWScoreSubsystem::SaveLobbyItemData(const FString& PlayerName, const FPTWLobbyItemData& LobbyItemData)
{
	if (!GetWorld() || !GetWorld()->GetAuthGameMode()) return;
	UE_LOG(LogTemp, Error, TEXT("Before Save PredictedPlayer: %s"), *LobbyItemData.PredictedData.PredictedPlayer);
	SavedLobbyItemData.Add(PlayerName, LobbyItemData);

	UE_LOG(LogTemp, Error, TEXT("Save PredictedPlayer: %s"), *SavedLobbyItemData.Find(PlayerName)->PredictedData.PredictedPlayer)
}

void UPTWScoreSubsystem::SaveGameRound(int32 NewGameRound)
{
	if (!GetWorld() || !GetWorld()->GetAuthGameMode()) return;
	SavedGameRound = NewGameRound;
}

void UPTWScoreSubsystem::SaveAllPlayerCount(int32 NewPlayerCount)
{
	if (!GetWorld() || !GetWorld()->GetAuthGameMode()) return;
	SavedAllPlayerCount = NewPlayerCount;
}

void UPTWScoreSubsystem::SaveGameData(const FPTWGameData& GameData)
{
	if (!GetWorld() || !GetWorld()->GetAuthGameMode()) return;
	SavedGameData = GameData;
}

void UPTWScoreSubsystem::IncreasePlayerCount()
{
	if (!GetWorld() || !GetWorld()->GetAuthGameMode()) return;
	++SavedAllPlayerCount;
}

void UPTWScoreSubsystem::DecreasePlayerCount()
{
	if (!GetWorld() || !GetWorld()->GetAuthGameMode()) return;
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






