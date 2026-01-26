// Fill out your copyright notice in the Description page of Project Settings.

#include "PTWSessionSubsystem.h"
#include "Kismet/GameplayStatics.h"

void UPTWSessionSubsystem::CreateLobbySession_Implementation(const TArray<FSessionPropertyKeyPair>& LobbySettings,
	int32 MaxPlayers, bool bisPrivate)
{
}

void UPTWSessionSubsystem::JoinLobbySession_Implementation(const FBlueprintSessionResult& SessionResult)
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

void UPTWSessionSubsystem::CreateListenServer(FName MapName)
{
	UGameplayStatics::OpenLevel(this, MapName, true, TEXT("listen"));
}
