// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWMiniGameMode.h"

#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "System/PTWScoreSubsystem.h"

class UPTWScoreSubsystem;

APTWMiniGameMode::APTWMiniGameMode()
{
	
}

void APTWMiniGameMode::BeginPlay()
{
	Super::BeginPlay();

	TravelLevelName = TEXT("/Game/_PTW/Maps/Lobby");

	if (PTWGameState)
	{
		PTWGameState->SetCurrentPhase(EPTWGamePhase::MiniGame);
	}
	
	StartTimer(MiniGameTime);
	
}

void APTWMiniGameMode::EndTimer()
{
	
	UE_LOG(LogTemp, Warning, TEXT("EndTimer PTWMiniGameMode"));

	Super::EndTimer();
}

void APTWMiniGameMode::AddWinPoint(APawn* PointPawn, int32 AddPoint)
{
	if (APTWPlayerState* PTWPlayerState = PointPawn->GetPlayerState<APTWPlayerState>())
	{
		//PTWPlayerState->AddWinPoint(AddPoint);
	}
}
