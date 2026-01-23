// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGameMode.h"

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
			PTWGameState->SetCurrentRound(PTWScoreSubsystem->GetCurrentGameRound()); // 현재 라운드 값 받아서 GameState에 전달
		}
	}
	
	StartTimer(60);

}

void APTWGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
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
	
	TravelLevel();
}

void APTWGameMode::TravelLevel()
{
	UE_LOG(LogTemp, Warning, TEXT("Travel Level"));
	GetWorld()->ServerTravel(TravelLevelName);
}

void APTWGameMode::UpdateTimer()
{
	if (PTWGameState)
	{
		PTWGameState->DecreaseTimer();
	}
}






