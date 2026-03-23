// Fill out your copyright notice in the Description page of Project Settings.

#include "PTWSessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "Algo/RandomShuffle.h"
#include "Kismet/GameplayStatics.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Online/OnlineSessionNames.h"
#include "System/Session/PTWSessionConfig.h"

bool UPTWSessionSubsystem::IsUsingSteamSubsystem()
{
	if (IOnlineSubsystem* SI = IOnlineSubsystem::Get())
	{
		return SI->GetSubsystemName() == FName("Steam");
	}
	return false;
}

FString UPTWSessionSubsystem::GetSteamServerID()
{
	if (SessionInterface.IsValid())
	{
		FNamedOnlineSession* NamedSession = SessionInterface->GetNamedSession(NAME_GameSession);
		if (NamedSession && NamedSession->SessionInfo.IsValid())
		{
			const FUniqueNetId& SessionId = NamedSession->SessionInfo->GetSessionId();
			FString ServerSteamIDStr = SessionId.ToString();
            
			return ServerSteamIDStr;
		}
	}
	
	return FString();
}

void UPTWSessionSubsystem::OnGameSessionActivated(FString InGameLiftSessionId)
{
	if (!SessionInterface.IsValid()) return;
	
	if (FOnlineSessionSettings* NewSettings = SessionInterface->GetSessionSettings(NAME_GameSession))
	{
		NewSettings->Set(FName("GameLiftSessionId"), InGameLiftSessionId, EOnlineDataAdvertisementType::ViaOnlineService);
		NewSettings->bShouldAdvertise = true;
		
		if (SessionInterface->UpdateSession(NAME_GameSession, *NewSettings, true))
		{
			UE_LOG(LogTemp, Log, TEXT("Session update requested."));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to start session update."));
		}
	}
}

void UPTWSessionSubsystem::CreateGameSession(FPTWSessionConfig SessionConfig, bool bTravelOnSuccess)
{
	if(!SessionInterface.IsValid()) return;
	SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	
    if (SessionInterface->GetNamedSession(NAME_GameSession))
    {
        // 이미 세션이 생성되어 있으면 세션 정리
        SessionInterface->DestroySession(NAME_GameSession);
    }
    CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
        FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete, SessionConfig, bTravelOnSuccess));
	
    TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
    SessionSettings->bIsLANMatch = false;											// Lan 연결 사용 여부
    SessionSettings->bShouldAdvertise = !SessionConfig.bIsDedicatedServer;			// 공개 여부: 검색 노출 및 친구 초대 가능
    SessionSettings->bAllowJoinInProgress = true;									// 중간 난입 허용 여부
	UE_LOG(LogTemp, Log, TEXT("bShouldAdvertise: %hs"), SessionSettings->bShouldAdvertise ? "true" : "false");
    SessionSettings->NumPublicConnections = SessionConfig.MaxPlayers;   
    
    SessionSettings->bIsDedicated = SessionConfig.bIsDedicatedServer;				// Dedicated Serer 여부
    SessionSettings->bUsesPresence = !SessionConfig.bIsDedicatedServer;				// 스팀 상태 정보(Presence)
    SessionSettings->bAllowJoinViaPresence = !SessionConfig.bIsDedicatedServer;		// 스팀 친구 참여
	SessionSettings->bUseLobbiesIfAvailable = !SessionConfig.bIsDedicatedServer;
	
	SessionSettings->Set(PTWSessionKey::MaxRounds, SessionConfig.MaxRounds, EOnlineDataAdvertisementType::ViaOnlineService);
    SessionSettings->Set(PTWSessionKey::ServerName, SessionConfig.ServerName, EOnlineDataAdvertisementType::ViaOnlineService);
    // SessionSettings->Set(PTWSessionKey::MaxPlayers, SessionConfig.MaxPlayers, EOnlineDataAdvertisementType::ViaOnlineService);
    
    if (!SessionInterface->CreateSession(0, NAME_GameSession, *SessionSettings))
    {
        // 생성 요청 실패, 델리게이트 해제
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
        UE_LOG(LogTemp, Warning, TEXT("CreateSession Call Failed"));
    }
}

void UPTWSessionSubsystem::JoinGameSession(const FOnlineSessionSearchResultBP& BPSearchResult)
{
	if (!SessionInterface.IsValid()) return;
	SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	const FOnlineSessionSearchResult& SearchResult = BPSearchResult.OnlineSessionSearchResult;
	
	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete));
	
	if (!SessionInterface->JoinSession(0, NAME_GameSession, SearchResult))
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		UE_LOG(LogTemp, Warning, TEXT("JoinSession Call Failed"));
	}
}

void UPTWSessionSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		UE_LOG(LogTemp, Error, TEXT("Join Session Failed! Result Code: %d"), (int32)Result);
		return;
	}
	if (!SessionInterface.IsValid()) return;
	
	SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	FString ConnectString;
	if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
	{
		if (APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController())
		{
			UE_LOG(LogTemp, Log, TEXT("Executing ClientTravel to: %s"), *ConnectString);
			PlayerController->ClientTravel(ConnectString, TRAVEL_Absolute);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to find LocalPlayerController for ClientTravel!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to resolve ConnectString! The session might be gone or Steam P2P failed."));
	}
}

void UPTWSessionSubsystem::FindGameSession()
{
	if(!SessionInterface.IsValid()) return;
	SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	
	SessionSearchQueue.Empty();
	BPSearchResults.Reset();
	
	TSharedPtr<FOnlineSessionSearch> ListenSessionSearch = MakeShareable(new FOnlineSessionSearch());
	ListenSessionSearch->bIsLanQuery = false;
	ListenSessionSearch->MaxSearchResults = 100;
	ListenSessionSearch->QuerySettings.Set(SEARCH_DEDICATED_ONLY, false, EOnlineComparisonOp::Equals);
	ListenSessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
	SessionSearchQueue.Enqueue(ListenSessionSearch);
	
	TSharedPtr<FOnlineSessionSearch> DedicatedSessionSearch = MakeShareable(new FOnlineSessionSearch());
	DedicatedSessionSearch->bIsLanQuery = false;
	DedicatedSessionSearch->MaxSearchResults = 100;
	DedicatedSessionSearch->QuerySettings.Set(SEARCH_DEDICATED_ONLY, true, EOnlineComparisonOp::Equals);
	DedicatedSessionSearch->QuerySettings.Set(SEARCH_LOBBIES, false, EOnlineComparisonOp::Equals);
	SessionSearchQueue.Enqueue(DedicatedSessionSearch);
	
	SearchForGameSessions();
}

void UPTWSessionSubsystem::SearchForGameSessions()
{
	if(!SessionInterface.IsValid()) return;
	SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	
	if (SessionSearchQueue.IsEmpty()) return;
	
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	SessionSearchQueue.Peek(SessionSearch);
	
	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)
	);
	
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef()))
	{
		UE_LOG(LogTemp, Log, TEXT("Session search started..."));
	}
	else
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		UE_LOG(LogTemp, Warning, TEXT("Failed to start session search."));
	}
}

void UPTWSessionSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	if(!SessionInterface.IsValid()) return;
	SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	
	if(SessionSearchQueue.IsEmpty()) return;
	
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	SessionSearchQueue.Dequeue(SessionSearch);
	
	if (bWasSuccessful && SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("Search Complete! Found %d sessions."), SessionSearch->SearchResults.Num());
		TArray<FOnlineSessionSearchResultBP> BPSearchResultInstances;
		for (const FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
		{
			FString SessionId = SearchResult.Session.GetSessionIdStr();
			FString HostName = SearchResult.Session.OwningUserName;
			int32 Ping = SearchResult.PingInMs;
			FString ServerName;
			
			if (SearchResult.Session.SessionSettings.Get(PTWSessionKey::ServerName, ServerName))
			{
				UE_LOG(LogTemp, Log, TEXT("Found Server: %s (Ping: %d)"), *ServerName, Ping);
				BPSearchResultInstances.Add(FOnlineSessionSearchResultBP(SearchResult));
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("Found Session: %s (Ping: %d)"), *SessionId, Ping);
			}
		}
		
		if (OnSessionSearchComplete.IsBound())
		{
			OnSessionSearchComplete.Broadcast(BPSearchResultInstances);
		}
		
		BPSearchResults += BPSearchResultInstances;
		if(SessionSearchQueue.IsEmpty())
		{
			if (OnAllSessionSearchFinished.IsBound())
			{
				OnAllSessionSearchFinished.Broadcast();
			}
		}
		else
		{
			SearchForGameSessions();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Session search failed."));
	}
}

void UPTWSessionSubsystem::OnQuickMatchFindSessionsComplete()
{
	if(!SessionInterface.IsValid()) return;
	OnAllSessionSearchFinished.RemoveDynamic(this, &ThisClass::OnQuickMatchFindSessionsComplete);

	TArray<FOnlineSessionSearchResultBP> BPAvailableSearchResults;
	for (FOnlineSessionSearchResultBP& BPSearchResult : BPSearchResults)
	{
		FOnlineSessionSearchResult& SessionData = BPSearchResult.OnlineSessionSearchResult;
		if (SessionData.IsValid() && SessionData.Session.NumOpenPublicConnections > 0)
		{
			BPAvailableSearchResults.Add(BPSearchResult);
		}
	}
	
	if (!BPAvailableSearchResults.IsEmpty())
	{
		Algo::RandomShuffle(BPAvailableSearchResults);
		for (FOnlineSessionSearchResultBP& BPAvailableSearchResult : BPAvailableSearchResults)
		{
			FOnlineSessionSearchResult& SessionData = BPAvailableSearchResult.OnlineSessionSearchResult;
			if (SessionData.IsValid() && SessionData.Session.NumOpenPublicConnections > 0)
			{
				JoinGameSession(BPAvailableSearchResult);
				return;
			}
		}
	}
	
	CreateGameSession(FPTWSessionConfig(), true);
}

void UPTWSessionSubsystem::LaunchDedicatedServer(FPTWSessionConfig SessionConfig)
{
	FString ServerPath = FPaths::ConvertRelativePathToFull(
		FPaths::Combine(
			FPaths::ProjectDir() + TEXT("Binaries/Win64/PTWServer.exe")));
	
	FString WorkingDir = FPaths::GetPath(ServerPath);
	FString ProjectPath = FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath());

	FString Arguments = FString::Printf(TEXT("\"%s\""), *ProjectPath);
	
	Arguments += FString::Printf(TEXT(" ServerEntry"));
	Arguments += FString::Printf(TEXT("?MaxPlayers=%d"), SessionConfig.MaxPlayers);
	
	Arguments += TEXT(" -Server");
	Arguments += TEXT(" -log");
	Arguments += TEXT(" -port=7777");
	Arguments += TEXT(" -QueryPort=27016");
	
	uint32 ProcessID = 0;
	
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
}

void UPTWSessionSubsystem::OpenServerLevel(FName MapName, FPTWSessionConfig SessionConfig)
{
	FString Options;
	if (!IsRunningDedicatedServer())
	{
		Options += FString::Printf(TEXT("?listen"));
	}
	
	// Options += FString::Printf(TEXT("?%s=%d"), *PTWSessionKey::MaxPlayers.ToString(), SessionConfig.MaxPlayers);
	Options += FString::Printf(TEXT("?%s=%d"), *PTWSessionKey::MaxRounds.ToString(), SessionConfig.MaxRounds);
	
	if (IsRunningDedicatedServer())
	{
		GetWorld()->ServerTravel(MapName.ToString() + Options);
	}
	else
	{
		UGameplayStatics::OpenLevel(this, MapName, true, Options);
	}
	
}

void UPTWSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	if (IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get())
	{
		SessionInterface = OnlineSubsystem->GetSessionInterface();
	}
	
	if (GEngine)
	{
		GEngine->OnNetworkFailure().AddUObject(this, &ThisClass::HandleNetworkFailure);
	}
	
	SessionSearchQueue.Empty();
	BPSearchResults.Reset();
}

void UPTWSessionSubsystem::Deinitialize()
{
	if (GEngine)
	{
		GEngine->OnNetworkFailure().RemoveAll(this);
	}
	SessionInterface = nullptr;
	
	Super::Deinitialize();
}

void UPTWSessionSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful, FPTWSessionConfig SessionConfig, bool bTravelOnSuccess)
{
	if(!SessionInterface.IsValid()) return;
	
	SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Log, TEXT("Steam Session Created Successfully!"));
		if (bTravelOnSuccess)
		{
			OpenServerLevel("lobby", SessionConfig);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to Create Steam Session."));
	}
}

void UPTWSessionSubsystem::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, 
	ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	UE_LOG(LogTemp, Log, TEXT("[PTWSessionSubsystem] HandleNetworkFailure() called"));
	LeaveGameSession();
}

void UPTWSessionSubsystem::LeaveGameSession()
{
	UE_LOG(LogTemp, Log, TEXT("[PTWSessionSubsystem] LeaveGameSession() called"));
	
	if (SessionInterface.IsValid())
	{
		DestroySessionDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
			FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete));
		
		SessionInterface->DestroySession(NAME_GameSession);
		return;
	}
	
	OnDestroySessionComplete(NAME_GameSession, true);
}

void UPTWSessionSubsystem::QuickMatchGameSession()
{
	if(!SessionInterface.IsValid()) return;
	OnAllSessionSearchFinished.AddUniqueDynamic(this, &UPTWSessionSubsystem::OnQuickMatchFindSessionsComplete);
	
	FindGameSession();
}

void UPTWSessionSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("[PTWSessionSubsystem] OnDestroySessionComplete() called"));
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionDelegateHandle);
	}
	
	UGameplayStatics::OpenLevel(this, FName("MainMenu"), true);
}
