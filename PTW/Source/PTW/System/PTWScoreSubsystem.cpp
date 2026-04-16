// Fill out your copyright notice in the Description page of Project Settings.


#include "System/PTWScoreSubsystem.h"

#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Game/GameInstance/PTWGameInstance.h"


void UPTWScoreSubsystem::SavePlayerGameData(const FString& PlayerID, const FPTWPlayerGameData& PlayerGameData)
{
	if (!GetWorld() || !GetWorld()->GetAuthGameMode()) return;
	ConnectedPlayersGameData.Add(PlayerID, PlayerGameData);
}

void UPTWScoreSubsystem::SaveServerTravelPlayerCount(int32 NewPlayerCount)
{
	if (!GetWorld() || !GetWorld()->GetAuthGameMode()) return;
	ServerTravelPlayerCount = NewPlayerCount;
}

void UPTWScoreSubsystem::SaveGameData(const FPTWGameData& GameData)
{
	if (!GetWorld() || !GetWorld()->GetAuthGameMode()) return;
	SavedGameData = GameData;
}

FPTWPlayerGameData* UPTWScoreSubsystem::FindPlayerGameData(const FString& PlayerId)
{
	return ConnectedPlayersGameData.Find(PlayerId);
}

void UPTWScoreSubsystem::RemoveTravelPlayersId()
{
	TravelPlayersId.Empty();
}

void UPTWScoreSubsystem::BeginPlay()
{
	
}

void UPTWScoreSubsystem::AddConnectedPlayerId(const FString& ConnectedPlayerId)
{
	ConnectedPlayersGameData.Add(ConnectedPlayerId);
}

void UPTWScoreSubsystem::AddTravelPlayerId(const FString& TravelPlayerId, const FString& PlayerName)
{
	if (!GetWorld() || !GetWorld()->GetAuthGameMode()) return;
	
	TravelPlayersId.Add(TravelPlayerId, PlayerName);
}






