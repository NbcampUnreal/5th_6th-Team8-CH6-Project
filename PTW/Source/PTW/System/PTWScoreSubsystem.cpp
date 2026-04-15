// Fill out your copyright notice in the Description page of Project Settings.


#include "System/PTWScoreSubsystem.h"

#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Game/GameInstance/PTWGameInstance.h"


void UPTWScoreSubsystem::SavePlayerData(const FString& PlayerID, const FPTWPlayerData& PlayerData)
{
	if (!GetWorld() || !GetWorld()->GetAuthGameMode()) return;
	SavedPlayersData.Add(PlayerID, PlayerData);
}

void UPTWScoreSubsystem::SaveLobbyItemData(const FString& PlayerID, const FPTWLobbyItemData& LobbyItemData)
{
	if (!GetWorld() || !GetWorld()->GetAuthGameMode()) return;
	SavedLobbyItemData.Add(PlayerID, LobbyItemData);
}

void UPTWScoreSubsystem::SavePlayerGameData(const FString& PlayerID, const FPTWPlayerGameData& PlayerGameData)
{
	if (!GetWorld() || !GetWorld()->GetAuthGameMode()) return;
	ConnectedPlayersGameData.Add(PlayerID, PlayerGameData);
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

FPTWPlayerGameData* UPTWScoreSubsystem::FindPlayerGameData(const FString& PlayerId)
{
	return ConnectedPlayersGameData.Find(PlayerId);
}

void UPTWScoreSubsystem::BeginPlay()
{
	UPTWGameInstance* GameInstance = Cast<UPTWGameInstance>(GetWorld()->GetGameInstance());
	if (!GameInstance) return;

	GameInstance->OnPlayerConnected.AddDynamic(this, &UPTWScoreSubsystem::AddConnectedPlayerId);
}

void UPTWScoreSubsystem::AddConnectedPlayerId(const FString& ConnectedPlayerId)
{
	ConnectedPlayersGameData.Add(ConnectedPlayerId);
}






