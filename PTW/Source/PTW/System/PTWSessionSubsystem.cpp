// Fill out your copyright notice in the Description page of Project Settings.

#include "PTWSessionSubsystem.h"

void UPTWSessionSubsystem::CreateLobbySession_Implementation(const FLobbySettings LobbySettings)
{
}

void UPTWSessionSubsystem::JoinLobbySession_Implementation(const FBlueprintSessionResult SessionResult)
{
}

void UPTWSessionSubsystem::FindLobbySession_Implementation()
{
}

void UPTWSessionSubsystem::OnFindLobbiesComplete(const TArray<FBlueprintSessionResult>& SessionResults)
{
	if (OnFindLobbiesCompleteDelegate.IsBound())
	{
		OnFindLobbiesCompleteDelegate.Broadcast(SessionResults);
	}
}
