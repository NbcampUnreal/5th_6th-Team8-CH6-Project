// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWServerEntryGameMode.h"
#include "System/PTWSessionSubsystem.h"

APTWServerEntryGameMode::APTWServerEntryGameMode()
{
	bUseSeamlessTravel = true;
}

void APTWServerEntryGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	if (!IsRunningDedicatedServer())
		return;
	
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	
	UPTWSessionSubsystem* SessionSubsystem = GI->GetSubsystem<UPTWSessionSubsystem>();
	if (!SessionSubsystem) return;
	
	FPTWSessionConfig SessionConfig;
	SessionConfig.ServerName = TEXT("MyDedicatedServer");
	SessionConfig.MaxPlayers = 16;
	
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, [SessionSubsystem, SessionConfig]()
	{
		SessionSubsystem->CreateGameSession(SessionConfig);
	}, 7.0f, false);
}

void APTWServerEntryGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}


