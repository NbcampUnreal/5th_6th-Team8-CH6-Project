// Fill out your copyright notice in the Description page of Project Settings.

#include "PTWSteamSessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "Algo/RandomShuffle.h"
#include "Kismet/GameplayStatics.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Online/OnlineSessionNames.h"
#include "System/Session/PTWSessionConfig.h"

#define LOCTEXT_NAMESPACE "STEAMSESSIONSUBSYSTEM"

UPTWSteamSessionSubsystem* UPTWSteamSessionSubsystem::Get(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (IsValid(World))
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			return GI->GetSubsystem<UPTWSteamSessionSubsystem>();
		}
	}
	return nullptr;
}

void UPTWSteamSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
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

void UPTWSteamSessionSubsystem::Deinitialize()
{
	if (GEngine)
	{
		GEngine->OnNetworkFailure().RemoveAll(this);
	}
	SessionInterface = nullptr;
	
	Super::Deinitialize();
}

bool UPTWSteamSessionSubsystem::IsUsingSteamSubsystem()
{
	if (IOnlineSubsystem* SI = IOnlineSubsystem::Get())
	{
		return SI->GetSubsystemName() == FName("Steam");
	}
	return false;
}

FString UPTWSteamSessionSubsystem::GetSteamServerID()
{
	if (SessionInterface.IsValid())
	{
		FNamedOnlineSession* CurrentGameSession = SessionInterface->GetNamedSession(NAME_GameSession);
		if (CurrentGameSession && CurrentGameSession->SessionInfo.IsValid())
		{
			const FUniqueNetId& SessionId = CurrentGameSession->SessionInfo->GetSessionId();
			FString ServerSteamIDStr = SessionId.ToString();
            
			return ServerSteamIDStr;
		}
	}
	
	return FString();
}

int32 UPTWSteamSessionSubsystem::GetMaxPlayers()
{
	if (SessionInterface.IsValid())
	{
		FNamedOnlineSession* CurrentGameSession = SessionInterface->GetNamedSession(NAME_GameSession);
		if (CurrentGameSession && CurrentGameSession->SessionInfo.IsValid())
		{
			return CurrentGameSession->NumOpenPublicConnections;
		}
	}
	
	return 16;
}

int32 UPTWSteamSessionSubsystem::GetMaxRounds()
{
	if (SessionInterface.IsValid())
	{
		FNamedOnlineSession* CurrentGameSession = SessionInterface->GetNamedSession(NAME_GameSession);
		if (CurrentGameSession && CurrentGameSession->SessionInfo.IsValid())
		{
			int32 RoundLimitTypeInt32;
			if (CurrentGameSession->SessionSettings.Get(FName(PTWSessionKey::MaxRounds), RoundLimitTypeInt32))
			{
				if (RoundLimitTypeInt32 == GetMaxRoundsByLimit(EPTWRoundLimit::Long))
				{
					return GetMaxRoundsByLimit(EPTWRoundLimit::Long);
				}
			}
		}
	}
	
	return GetMaxRoundsByLimit(EPTWRoundLimit::Short);
}

void UPTWSteamSessionSubsystem::CreateGameSession(FPTWSessionConfig SessionConfig, bool bTravelOnSuccess)
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
    SessionSettings->bShouldAdvertise = true;										// 공개 여부: 검색 노출 및 친구 초대 가능
    SessionSettings->bAllowJoinInProgress = true;									// 중간 난입 허용 여부
    SessionSettings->NumPublicConnections = SessionConfig.MaxPlayers;   
    
    SessionSettings->bIsDedicated = SessionConfig.bIsDedicatedServer;				// Dedicated Serer 여부
    SessionSettings->bUsesPresence = !SessionConfig.bIsDedicatedServer;				// 스팀 상태 정보(Presence)
    SessionSettings->bAllowJoinViaPresence = !SessionConfig.bIsDedicatedServer;		// 스팀 친구 참여
	SessionSettings->bUseLobbiesIfAvailable = !SessionConfig.bIsDedicatedServer;
	
	SessionSettings->Set(PTWSessionKey::MaxRounds, SessionConfig.MaxRounds, EOnlineDataAdvertisementType::ViaOnlineService);
    SessionSettings->Set(PTWSessionKey::ServerName, SessionConfig.ServerName, EOnlineDataAdvertisementType::ViaOnlineService);
	
	if (SessionConfig.bIsNoGameLift)
	{
		SessionSettings->Set(PTWSessionKey::NoGameLift, SessionConfig.bIsNoGameLift, EOnlineDataAdvertisementType::ViaOnlineService);
	}
    
    if (!SessionInterface->CreateSession(0, NAME_GameSession, *SessionSettings))
    {
        // 생성 요청 실패, 델리게이트 해제
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
        UE_LOG(LogTemp, Warning, TEXT("CreateSession Call Failed"));
    	if (OnSteamSessionMessageReceived.IsBound())
    	{
    		FText ErrorMessage = LOCTEXT("SessionCreateFailed", "알 수 없는 오류가 발생해 세션 생성에 실패했습니다.");
    		OnSteamSessionMessageReceived.Broadcast(ErrorMessage);
    	}
    }
}

void UPTWSteamSessionSubsystem::JoinGameSession(const FOnlineSessionSearchResultBP& BPSearchResult)
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

void UPTWSteamSessionSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
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

void UPTWSteamSessionSubsystem::FindGameSession()
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

void UPTWSteamSessionSubsystem::SearchForGameSessions()
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

void UPTWSteamSessionSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
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
			BPSearchResultInstances.Add(FOnlineSessionSearchResultBP(SearchResult));
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

void UPTWSteamSessionSubsystem::OnQuickMatchFindSessionsComplete()
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

void UPTWSteamSessionSubsystem::LaunchDedicatedServer(FPTWSessionConfig SessionConfig)
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

void UPTWSteamSessionSubsystem::OpenServerLevel(FName MapName, FPTWSessionConfig SessionConfig)
{
	FString Options;
	if (!IsRunningDedicatedServer())
	{
		Options += FString::Printf(TEXT("?listen"));
	}
	
	// Options += FString::Printf(TEXT("?%s=%d"), *PTWSessionKey::MaxPlayers.ToString(), SessionConfig.MaxPlayers);
	// Options += FString::Printf(TEXT("?%s=%d"), *PTWSessionKey::MaxRounds.ToString(), SessionConfig.MaxRounds);
	
	if (IsRunningDedicatedServer())
	{
		GetWorld()->ServerTravel(MapName.ToString() + Options);
	}
	else
	{
		UGameplayStatics::OpenLevel(this, MapName, true, Options);
	}
	
}

void UPTWSteamSessionSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful, FPTWSessionConfig SessionConfig, bool bTravelOnSuccess)
{
	if(!SessionInterface.IsValid()) return;
	
	SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(DestroySessionDelegateHandle);
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

void UPTWSteamSessionSubsystem::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, 
	ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	UE_LOG(LogTemp, Log, TEXT("[PTWSessionSubsystem] HandleNetworkFailure() called"));
	LeaveGameSession();
}

void UPTWSteamSessionSubsystem::LeaveGameSession()
{
	UE_LOG(LogTemp, Log, TEXT("[PTWSessionSubsystem] LeaveGameSession() called"));
	
	// 메인메뉴로 이동하여 이탈
	if (APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController())
	{
		PC->ClientTravel(TEXT("MainMenu?closed"), TRAVEL_Absolute);
	}
	
	if (SessionInterface.IsValid())
	{
		DestroySessionDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
			FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete));
		
		SessionInterface->DestroySession(NAME_GameSession);
		return;
	}
	
	OnDestroySessionComplete(NAME_GameSession, true);
}

bool UPTWSteamSessionSubsystem::UnregisterPlayer(FName SessionName, const FUniqueNetId& PlayerId)
{
	if (SessionInterface.IsValid())
	{
		return SessionInterface->UnregisterPlayer(NAME_GameSession, PlayerId);
		// UE_LOG(LogTemp, Log, TEXT("Player unregistered from session to clear ghost slot."));
	}
	else
	{
		return false;
	}
}

void UPTWSteamSessionSubsystem::ExitGameSession()
{
	if (SessionInterface.IsValid())
	{
		if (SessionInterface.IsValid())
		{
			SessionInterface->DestroySession(NAME_GameSession); 
			UE_LOG(LogTemp, Log, TEXT("Server's Steam Session Destroyed."));
		}
	}
}

void UPTWSteamSessionSubsystem::QuickMatchGameSession()
{
	if(!SessionInterface.IsValid()) return;
	OnAllSessionSearchFinished.AddUniqueDynamic(this, &UPTWSteamSessionSubsystem::OnQuickMatchFindSessionsComplete);
	
	FindGameSession();
}

void UPTWSteamSessionSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("[PTWSessionSubsystem] OnDestroySessionComplete() called"));
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionDelegateHandle);
	}
	
	UGameplayStatics::OpenLevel(this, FName("MainMenu"), true);
}
#undef LOCTEXT_NAMESPACE
