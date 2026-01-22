// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWMiniGameMode.h"

void APTWMiniGameMode::BeginPlay()
{
	Super::BeginPlay();

	TravelLevelName = TEXT("/Game/Developers/wonjun/TestLobby");
	
	StartTimer(MiniGameTime);
	
}
