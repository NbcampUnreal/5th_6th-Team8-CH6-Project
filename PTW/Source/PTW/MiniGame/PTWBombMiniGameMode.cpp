// Fill out your copyright notice in the Description page of Project Settings.

#include "PTWBombMiniGameMode.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"

void APTWBombMiniGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentRound = 0;
	StartRound();
}

void APTWBombMiniGameMode::StartRound()
{
	CurrentRound++;

	UE_LOG(LogTemp, Warning, TEXT("[BombMode] Round %d / %d - Countdown Start"), CurrentRound, MaxRoundCount);
	
	if (APTWGameState* GS = GetGameState<APTWGameState>())
	{
		GS->SetbMiniGameCountdown(true);
	}
	
	StartCountDown();
}

void APTWBombMiniGameMode::OnCountDownFinished()
{
	if (APTWGameState* GS = GetGameState<APTWGameState>())
	{
		GS->SetbMiniGameCountdown(false);
	}

	UE_LOG(LogTemp, Warning, TEXT("[BombMode] Round %d - Play Start"), CurrentRound);

	// 라운드(폭탄) 진행 타이머 시작 
	GetWorldTimerManager().SetTimer(
	RoundTimerHandle,
	this,
	&APTWBombMiniGameMode::EndTimer,
	RoundPlayTime,
	false
);
}

void APTWBombMiniGameMode::EndTimer()
{
	UE_LOG(LogTemp, Warning, TEXT("[BombMode] Round %d - Explosion Timing"), CurrentRound);
	

	// 3회 다 돌면 미니게임 종료
	if (CurrentRound >= MaxRoundCount)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BombMode] Finished"));
		
		Super::EndTimer();
		return;
	}

	// 다음 라운드 진행
	StartRound();
}
