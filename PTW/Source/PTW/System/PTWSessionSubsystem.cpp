// Fill out your copyright notice in the Description page of Project Settings.

#include "PTWSessionSubsystem.h"

#include "BlueprintDataDefinitions.h"
#include "Kismet/GameplayStatics.h"
#include "GenericPlatform/GenericPlatformProcess.h"

void UPTWSessionSubsystem::CreateLobbySession_Implementation(const TArray<FSessionPropertyKeyPair>& LobbySettings,
	int32 MaxPlayers, bool bIsPrivate)
{
}

void UPTWSessionSubsystem::JoinLobbySession_Implementation(const FBlueprintSessionResult& SessionResult)
{
}

void UPTWSessionSubsystem::FindLobbySession_Implementation()
{
}

void UPTWSessionSubsystem::OnFindLobbiesComplete(const TArray<FBlueprintSessionResult>& SessionResults)
{
	if (OnFindLobbiesCompleteDelegate.IsBound())
	{
		OnFindLobbiesCompleteDelegate.Broadcast(SessionResults);
	}
}

void UPTWSessionSubsystem::LaunchDedicatedServer(const TArray<FSessionPropertyKeyPair>& LobbySettings,
	int32 MaxPlayers, bool bIsPrivate)
{
	// FString ServerPath = FPaths::ConvertRelativePathToFull(FPaths::RootDir() + 
	// 	TEXT("Engine/Binaries/Win64/UnrealEditor.exe"));
	
	FString ServerPath = FPaths::ConvertRelativePathToFull(
		FPaths::Combine(
			FPaths::ProjectDir() + TEXT("Binaries/Win64/PTWServer.exe")));
	
	FString WorkingDir = FPaths::GetPath(ServerPath);
	FString ProjectPath = FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath());

	FString Arguments = FString::Printf(TEXT("\"%s\""), *ProjectPath);
	
	Arguments += FString::Printf(TEXT(" ServerEntry"));
	Arguments += FString::Printf(TEXT("?MaxPlayers=%d"), MaxPlayers);
	Arguments += FString::Printf(TEXT("?bIsPrivate=%d"), bIsPrivate ? 1 : 0);
	
	for (const FSessionPropertyKeyPair& LobbySetting : LobbySettings)
	{
		// FName Key;
		// FVariantData Data;
		FString Key = LobbySetting.Key.ToString();
		FString Value = LobbySetting.Data.ToString();
		Arguments += FString::Printf(TEXT("?%s=\"%s\""), *Key, *Value);
	}
	
	// Arguments += TEXT(" -NoSteam");
	Arguments += TEXT(" -Server");
	Arguments += TEXT(" -log");
	Arguments += TEXT(" -port=7777");
	Arguments += TEXT(" -QueryPort=27016");
	
	
	uint32 ProcessID = 0;
	
	// FString AppID = TEXT("480");
	// FPlatformMisc::SetEnvironmentVar(TEXT("SteamAppId"), *AppID);
	
	// FPlatformMisc::SetEnvironmentVar(TEXT("SteamGameId"), *AppID);
	// FPlatformMisc::SetEnvironmentVar(TEXT("IsDedicatedServerContext"), TEXT("True"));
	
	FProcHandle ProcHandle = FPlatformProcess::CreateProc(
		*ServerPath,	// 데디케이티드 서버 실행파일 경로
		*Arguments, 	// 명령인자
		true,  			// 독립 실행 (게임 클라이언가 종료되어도 데디케이티드 서버는 닫히지 않음)
		false, 			// 데디케이티드 서버 창 숨김
		false, 			// 완전 숨김. 백그라운드에서 구동되고 표시되지 않음
		&ProcessID, 	// 프로세스 ID. 인자에 저장
		0, 				// 우선순위. (0:기본값, -1:낮음, 1:높음)
		*WorkingDir, 		// 작업 디렉터리(nullptr: 실행파일 위치를 작업디렉터리로 인식)
		nullptr			// 프로세스간 통신 출력단.
	);

	if (ProcHandle.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("Server Create Succeeded! PID: %d"), ProcessID);
		FPlatformProcess::CloseProc(ProcHandle);
	}
	// FTimerHandle TimerHandle;
	// GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
	// {
	// 	GetWorld()->GetFirstPlayerController()->ClientTravel("127.0.0.1:7777", TRAVEL_Absolute);
	// }, 15.0f, false);
}

void UPTWSessionSubsystem::CreateListenLevel(FName MapName)
{
	UGameplayStatics::OpenLevel(this, MapName, true, TEXT("listen"));
}
