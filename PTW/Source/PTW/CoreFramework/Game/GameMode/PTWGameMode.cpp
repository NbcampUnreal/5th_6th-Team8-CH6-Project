// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGameMode.h"

#include "CoreFramework/PTWPlayerState.h"
#include "PTW/CoreFramework/Game/GameState/PTWGameState.h"
#include "System/PTWScoreSubsystem.h"


APTWGameMode::APTWGameMode()
{
	bUseSeamlessTravel = true;
}

void APTWGameMode::BeginPlay()
{
	Super::BeginPlay();

	PTWGameState = GetGameState<APTWGameState>();

	if (PTWGameState)
	{
		PTWGameState->OnTimerFinished.AddDynamic(this, &APTWGameMode::EndTimer);

		if (UPTWScoreSubsystem* PTWScoreSubsystem = GetGameInstance()->GetSubsystem<UPTWScoreSubsystem>())
		{
			PTWGameState->SetCurrentRound(PTWScoreSubsystem->GetCurrentGameRound()); // GameInstance 라운드 값 받아서 GameState에 전달
		}
	}
	
}

void APTWGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ApplyPlayerDataFromSubsystem(NewPlayer);
}

void APTWGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	// 접속 해제 시 게임 참여 인원 감소
}

void APTWGameMode::StartTimer(float TimeDuration)
{
	if (PTWGameState)
	{
		PTWGameState->SetRemainTime(TimeDuration);
	}
	
	GetWorldTimerManager().SetTimer(TimerHandle, this, &APTWGameMode::UpdateTimer, 1.f, true, 1.f);
}

void APTWGameMode::EndTimer()
{
	GetWorldTimerManager().ClearTimer(TimerHandle);

	SaveGameDataToSubsystem();
	
	TravelLevel();
}

void APTWGameMode::TravelLevel()
{
	UE_LOG(LogTemp, Warning, TEXT("Travel Level"));
	GetWorld()->ServerTravel(TravelLevelName);
}

void APTWGameMode::SaveGameDataToSubsystem()
{
	if (!PTWGameState) return;
	
	if (UPTWScoreSubsystem* PTWScoreSubsystem = GetGameInstance()->GetSubsystem<UPTWScoreSubsystem>())
	{
		PTWScoreSubsystem->SaveGameRound(PTWGameState->GetCurrentRound());

		for (APlayerState* PlayerState : PTWGameState->PlayerArray)
		{
			if (APTWPlayerState* PTWPlayerState = Cast<APTWPlayerState>(PlayerState))
			{
				PTWScoreSubsystem->SavePlayerData(PTWPlayerState->GetPlayerId(), PTWPlayerState->GetPlayerData());
			}
		}
	}
}

void APTWGameMode::ApplyPlayerDataFromSubsystem(APlayerController* NewPlayer)
{
	if (!PTWGameState) return;
	
	if (UPTWScoreSubsystem* PTWScoreSubsystem = GetGameInstance()->GetSubsystem<UPTWScoreSubsystem>())
	{
		if (APTWPlayerState* PTWPlayerState = NewPlayer->GetPlayerState<APTWPlayerState>())
		{
			if (FPTWPlayerData* FoundData = PTWScoreSubsystem->FindPlayerData(PTWPlayerState->GetPlayerId()))
			{
				PTWPlayerState->SetPlayerData(*FoundData);
			}
		}
	}
}

void APTWGameMode::UpdateTimer()
{
	if (PTWGameState)
	{
		PTWGameState->DecreaseTimer();
	}
}






