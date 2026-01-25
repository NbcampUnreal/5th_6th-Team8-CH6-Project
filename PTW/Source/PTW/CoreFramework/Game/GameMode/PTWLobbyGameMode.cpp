// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWLobbyGameMode.h"

#include "PTW/CoreFramework/Game/GameState/PTWGameState.h"
#include "System/PTWScoreSubsystem.h"

void APTWLobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	TravelLevelName = TEXT("/Game/_PTW/Maps/MiniGame_Bomb");

	if (PTWGameState)
	{
		if (UPTWScoreSubsystem* PTWScoreSubsystem = GetGameInstance()->GetSubsystem<UPTWScoreSubsystem>())
		{
			if (PTWScoreSubsystem->bIsFirstLobby == true)
			{
				PTWGameState->SetCurrentPhase(EPTWGamePhase::PreGameLobby);
				PTWScoreSubsystem->bIsFirstLobby = false;
			}
			else
			{
				PTWGameState->SetCurrentPhase(EPTWGamePhase::PostGameLobby);
				PTWGameState->AdvanceRound(); // 라운드 증가
			}
		}
	}
	
	StartTimer(LobbyWaitingTime);
}

void APTWLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	//접속하면 무적 상태로 변경

	if (PTWGameState)
	{
		if (PTWGameState->GetCurrentGamePhase() == EPTWGamePhase::PreGameLobby)
		{
			AddRandomGold();
		}
		
	}
}

void APTWLobbyGameMode::StartMiniGame()
{
	//TravelLevel();
}

void APTWLobbyGameMode::AddRandomGold()
{
	
}
