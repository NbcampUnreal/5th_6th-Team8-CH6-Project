// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWLobbyGameMode.h"

#include "CoreFramework/PTWPlayerState.h"
#include "PTW/CoreFramework/Game/GameState/PTWGameState.h"
#include "System/PTWScoreSubsystem.h"



void APTWLobbyGameMode::InitGameState()
{
	Super::InitGameState();

	TravelLevelName = TEXT("/Game/_PTW/Maps/MiniGame_Bomb");
	
	PTWGameState = GetGameState<APTWGameState>();
	
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

void APTWLobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	// TravelLevelName = TEXT("/Game/_PTW/Maps/MiniGame_Bomb");
	//
	// if (PTWGameState)
	// {
	// 	if (UPTWScoreSubsystem* PTWScoreSubsystem = GetGameInstance()->GetSubsystem<UPTWScoreSubsystem>())
	// 	{
	// 		if (PTWScoreSubsystem->bIsFirstLobby == true)
	// 		{
	// 			PTWGameState->SetCurrentPhase(EPTWGamePhase::PreGameLobby);
	// 			
	// 			PTWScoreSubsystem->bIsFirstLobby = false;
	// 		}
	// 		else
	// 		{
	// 			PTWGameState->SetCurrentPhase(EPTWGamePhase::PostGameLobby);
	// 			
	// 			PTWGameState->AdvanceRound(); // 라운드 증가
	// 		}
	// 	}
	// }
	//UE_LOG(LogTemp, Warning, TEXT("Beginplay "));
	
}

void APTWLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	//접속하면 무적 상태로 변경
	
	if (!IsValid(PTWGameState)) return;
	UE_LOG(LogTemp, Warning, TEXT("PostLogin "));
	if (PTWGameState->GetCurrentGamePhase() == EPTWGamePhase::PreGameLobby)
	{
		AddRandomGold(NewPlayer);
	}
}

void APTWLobbyGameMode::AddRandomGold(APlayerController* NewPlayer)
{
	int32 RandomGold = FMath::RandRange(1, 100);

	if (APTWPlayerState* PTWPlayerState = NewPlayer->GetPlayerState<APTWPlayerState>())
	{
		FPTWPlayerData PlayerData;
		PlayerData.Gold = RandomGold;
		PTWPlayerState->SetPlayerData(PlayerData);

		UE_LOG(LogTemp, Warning, TEXT("RandomGold: %d"), RandomGold);
	}
}

void APTWLobbyGameMode::StartMiniGame()
{
	//TravelLevel();
}

