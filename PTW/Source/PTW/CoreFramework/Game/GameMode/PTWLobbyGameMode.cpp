// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWLobbyGameMode.h"

#include "CoreFramework/PTWPlayerState.h"
#include "PTW/CoreFramework/Game/GameState/PTWGameState.h"
#include "System/PTWScoreSubsystem.h"


void APTWLobbyGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	
	if (UPTWScoreSubsystem* PTWScoreSubsystem = GetGameInstance()->GetSubsystem<UPTWScoreSubsystem>())
	{
		if (PTWScoreSubsystem->bIsFirstLobby == true)
		{
			bIsFirstLobby = true;	
			PTWScoreSubsystem->bIsFirstLobby = false;
		}
		else
		{
			bIsFirstLobby = false;	
		}
	}
}

void APTWLobbyGameMode::InitGameState()
{
	Super::InitGameState();

	TravelLevelName = TEXT("/Game/_PTW/Maps/MiniGame_Bomb");

	if (IsValid(PTWGameState))
	{
		if (bIsFirstLobby == true)
		{
			PTWGameState->SetCurrentPhase(EPTWGamePhase::PreGameLobby);
		}
		else
		{
			PTWGameState->SetCurrentPhase(EPTWGamePhase::PostGameLobby);
			PTWGameState->AdvanceRound(); // 라운드 증가
		}
	}
	
}

void APTWLobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

}

void APTWLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	//접속하면 무적 상태로 변경 해야 함
	
	if (!IsValid(PTWGameState)) return;
	
	if (PTWGameState->GetCurrentGamePhase() == EPTWGamePhase::PreGameLobby)
	{
		AddRandomGold(NewPlayer);
		
		UE_LOG(LogTemp, Warning, TEXT("Login Players :%d"), PTWGameState->PlayerArray.Num());
		// PreGameLobby 상태에서 최소 인원 충족 되면 WaitingTimer 시작
		if (PTWGameState->PlayerArray.Num() >= GameFlowRule.MinPlayersToStart)
		{
			if (bWaitingTimerStarted == false)
			{
				StartTimer(GameFlowRule.WaitingTime);
				bWaitingTimerStarted = true;
			}
		}
	}
	
}

void APTWLobbyGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	if (!IsValid(PTWGameState)) return;

	if (PTWGameState->GetCurrentGamePhase() == EPTWGamePhase::PostGameLobby)
	{
		// 로딩 UI 
		if (PTWGameState->PlayerArray.Num() == 1) // 임시 설정
		{
			// 플레이 중인 모든 플레이어 접속 중이면 로딩 UI 해제
			
		}
		StartTimer(GameFlowRule.NextMiniGameWaitTime);
	}
}

void APTWLobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	
	if (!IsValid(PTWGameState)) return;

	if (PTWGameState->GetCurrentGamePhase() == EPTWGamePhase::PreGameLobby)
	{
		if (PTWGameState->PlayerArray.Num() < GameFlowRule.MinPlayersToStart)
		{
			if (bWaitingTimerStarted == true)
			{
				ClearTimer();
				bWaitingTimerStarted = false;
			}
		}
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


