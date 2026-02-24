// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWServerEntryGameMode.h"
#include "CoreFramework/Game/GameSession/PTWGameSession.h"

APTWServerEntryGameMode::APTWServerEntryGameMode()
{
	bUseSeamlessTravel = true;
	GameSessionClass = APTWGameSession::StaticClass();
}
