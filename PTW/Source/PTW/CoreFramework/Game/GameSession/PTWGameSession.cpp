// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGameSession.h"
#include "System/PTWSessionSubsystem.h"
#include "System/Session/PTWSessionConfig.h"

void APTWGameSession::RegisterServer()
{
	Super::RegisterServer();
	
	if (IsRunningDedicatedServer())
	{
		UPTWSessionSubsystem* SessionSubsystem = GetGameInstance()->GetSubsystem<UPTWSessionSubsystem>();
		if (SessionSubsystem)
		{
			FPTWSessionConfig SessionConfig;
			SessionConfig.ServerName = TEXT("MyDedicatedServer");
			SessionConfig.MaxPlayers = 16;
			SessionConfig.bIsDedicatedServer = UE_SERVER;
			
			const TCHAR* CommandLine = FCommandLine::Get();
			
			FString ServerName_cmd = FString::Printf(TEXT("-%s="), *PTWSessionKey::ServerName.ToString());
			FString MaxPlayers_cmd = FString::Printf(TEXT("-%s="), *PTWSessionKey::MaxPlayers.ToString());
			
			FParse::Value(CommandLine, *ServerName_cmd, SessionConfig.ServerName);
			FParse::Value(CommandLine, *MaxPlayers_cmd, SessionConfig.MaxPlayers);
			
			SessionSubsystem->CreateGameSession(SessionConfig);
		}
	}
}
