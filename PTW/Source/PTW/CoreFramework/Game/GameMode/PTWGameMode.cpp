// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGameMode.h"

#include "PTW/CoreFramework/Game/GameState/PTWGameState.h"


APTWGameMode::APTWGameMode()
{
	bUseSeamlessTravel = true;
}

void APTWGameMode::BeginPlay()
{
	Super::BeginPlay();

	//PTWGameState = GetGameState<APTWGameState>();
}

void APTWGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// 플레이어 접속 시 게임 참여 인원 증가

	//FTimerHandle TestHandle;
	//GetWorldTimerManager().SetTimer(TestHandle, this, &APTWGameMode::TravelLevel, 10.f, false);
}

void APTWGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	// 접속 해제 시 게임 참여 인원 감소
}

void APTWGameMode::TravelLevel()
{
	//GetWorld()->ServerTravel("/Game/Developers/wonjun/TestMini");
}






