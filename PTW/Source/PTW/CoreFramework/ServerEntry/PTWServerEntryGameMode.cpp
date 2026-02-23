// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWServerEntryGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "System/PTWSessionSubsystem.h"

void APTWServerEntryGameMode::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Display, TEXT("PTWMainMenuGameMode BeginPlay"));
	
	if (IsRunningDedicatedServer())
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UPTWSessionSubsystem* SessionSubsystem = GI->GetSubsystem<UPTWSessionSubsystem>())
			{
				FPTWSessionConfig SessionConfig;
				SessionConfig.ServerName = TEXT("데디케이티드 서버");
				SessionConfig.MaxPlayers = 16;
				FTimerHandle TimerHandle;
				GetWorldTimerManager().SetTimer(TimerHandle, [SessionSubsystem, SessionConfig]()
				{
					SessionSubsystem->CreateGameSession(SessionConfig);
				}, 5.0f, false);
			}
		}
	}
}




