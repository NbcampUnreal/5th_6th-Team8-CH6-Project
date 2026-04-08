#include "PTWSteamSessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Online/OnlineSessionNames.h"
#include "System/Session/PTWSessionConfig.h"
#include "Debug/PTWLogCategorys.h"

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

FString UPTWSteamSessionSubsystem::GetSteamServerID() const
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
	
	// 이미 생성된 세션이 존재할 경우 파괴
    if (SessionInterface->GetNamedSession(NAME_GameSession))
    {
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
	
	if (!SessionConfig.bIsDedicatedServer)
	{
		SessionSettings->Set(PTWSessionKey::MaxRounds, SessionConfig.MaxRounds, EOnlineDataAdvertisementType::ViaOnlineService);
		SessionSettings->Set(PTWSessionKey::ServerName, SessionConfig.ServerName, EOnlineDataAdvertisementType::ViaOnlineService);
	}
	else if (SessionConfig.bIsNoGameLift)
	{
		SessionSettings->Set(PTWSessionKey::NoGameLift, SessionConfig.bIsNoGameLift, EOnlineDataAdvertisementType::ViaOnlineService);
	}
    
    if (SessionInterface->CreateSession(0, NAME_GameSession, *SessionSettings))
    {
    	UE_LOG(Log_Steam, Display, TEXT("[게임세션 생성요청] 스팀게임세션 생성요청 전송완료"));
    }
	else
    {
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		
		// 서버는 AlarmUI 미표시
		#if !UE_SERVER
    	if (OnSteamSessionMessageReceived.IsBound())
    	{
    		FText ErrorMessage = LOCTEXT("SessionCreateFailed", "알 수 없는 오류가 발생해 세션 생성에 실패했습니다.");
    		OnSteamSessionMessageReceived.Broadcast(ErrorMessage);
    	}
		#endif
    	UE_LOG(Log_Steam, Error, TEXT("[게임세션 생성요청] 스팀게임세션 생성요청 전송실패"));
    }
}

void UPTWSteamSessionSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful, FPTWSessionConfig SessionConfig, bool bTravelOnSuccess)
{
	if(!SessionInterface.IsValid()) return;
	
	SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(DestroySessionDelegateHandle);
	if (bWasSuccessful)
	{
		UE_LOG(Log_Steam, Display, TEXT("[게임세션 생성응답] 스팀게임세션 생성성공 응답"));
		if (bTravelOnSuccess)
		{
			OpenServerLevel("lobby", SessionConfig);
		}
	}
	else
	{
		UE_LOG(Log_Steam, Error, TEXT("[게임세션 생성응답] 스팀게임세션 생성실패 응답"));
	}
}

void UPTWSteamSessionSubsystem::JoinGameSession(const FOnlineSessionSearchResultBP& BPSearchResult, const FString Options)
{
	if (!SessionInterface.IsValid()) return;
	SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	const FOnlineSessionSearchResult& SearchResult = BPSearchResult.OnlineSessionSearchResult;
	
	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete, Options));
	
	if (SessionInterface->JoinSession(0, NAME_GameSession, SearchResult))
	{
		UE_LOG(Log_Steam, Display, TEXT("[게임세션 접속요청] 스팀게임세션 접속요청 전송완료"));
	}
	else
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		if (OnSteamSessionMessageReceived.IsBound())
		{
			FText ErrorMessage = LOCTEXT("JoinSteamSessionFailed", "스팀세션 접속에 실패했습니다.");
			OnSteamSessionMessageReceived.Broadcast(ErrorMessage);
		}
		UE_LOG(Log_Steam, Error, TEXT("[게임세션 접속요청] 스팀게임세션 접속요청 전송실패"));
	}
}

void UPTWSteamSessionSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result, const FString Options)
{
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		UE_LOG(Log_Steam, Error, TEXT("[게임세션 접속응답] 스팀게임세션 접속실패 응답 (Code: %d)"), (int32)Result);
		return;
	}
	if (!SessionInterface.IsValid()) return;
	
	SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	FString ConnectString;
	if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
	{
		ConnectString += Options; 
		if (APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController())
		{
			PlayerController->ClientTravel(ConnectString, TRAVEL_Absolute);
		}
	}
	else
	{
		UE_LOG(Log_Steam, Error, TEXT("게임세션 접속실패] 세션 접속 정보 해석 실패. 유효하지 않은 세션 또는 네트워크 상태 확인"));
	}
}

void UPTWSteamSessionSubsystem::FindGameSession()
{
	if(!SessionInterface.IsValid()) return;
	SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	
	SessionSearchQueue.Empty();
	BPSearchResults.Reset();
	
	// TSharedPtr<FOnlineSessionSearch> ListenSessionSearch = MakeShareable(new FOnlineSessionSearch());
	// ListenSessionSearch->bIsLanQuery = false;
	// ListenSessionSearch->MaxSearchResults = 100;
	// ListenSessionSearch->QuerySettings.Set(SEARCH_DEDICATED_ONLY, false, EOnlineComparisonOp::Equals);
	// ListenSessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
	// SessionSearchQueue.Enqueue(ListenSessionSearch);
	
	TSharedPtr<FOnlineSessionSearch> DedicatedSessionSearch = MakeShareable(new FOnlineSessionSearch());
	DedicatedSessionSearch->bIsLanQuery = false;
	DedicatedSessionSearch->MaxSearchResults = 100;
	DedicatedSessionSearch->QuerySettings.Set(SEARCH_DEDICATED_ONLY, true, EOnlineComparisonOp::Equals);
	DedicatedSessionSearch->QuerySettings.Set(SEARCH_LOBBIES, false, EOnlineComparisonOp::Equals);
	DedicatedSessionSearch->QuerySettings.Set(PTWSessionKey::NoGameLift, true, EOnlineComparisonOp::Equals);
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
		UE_LOG(Log_Steam, Display, TEXT("[게임세션 탐색요청] 스팀게임세션 탐색요청 전송완료"));
	}
	else
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		UE_LOG(Log_Steam, Error, TEXT("[게임세션 탐색요청] 스팀게임세션 탐색요청 전송실패"));
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
		UE_LOG(Log_Steam, Display, TEXT("[게임세션 탐색응답] 스팀게임세션 탐색성공 응답"));
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
		
		// 마지막 탐색에서 리스트 반환
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
		UE_LOG(Log_Steam, Error, TEXT("[게임세션 탐색응답] 스팀게임세션 탐색실패 응답"));
	}
}

void UPTWSteamSessionSubsystem::LeaveGameSession()
{
	if (!SessionInterface.IsValid()) return;
	
	DestroySessionDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
		FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete));
		
	if (SessionInterface->DestroySession(NAME_GameSession))
	{
		UE_LOG(Log_Steam, Display, TEXT("[게임세션 파괴요청] 스팀게임세션 파괴요청 전송완료"));
	}
	else
	{
		UE_LOG(Log_Steam, Error, TEXT("[게임세션 파괴요청] 스팀게임세션 파괴요청 전송실패"));
		OnDestroySessionComplete(NAME_GameSession, false);
	}
}

void UPTWSteamSessionSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionDelegateHandle);
	}
	
	if (APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController())
	{
		PC->ClientTravel(TEXT("MainMenu?closed"), TRAVEL_Absolute);
	}
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

void UPTWSteamSessionSubsystem::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, 
	ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	UE_LOG(Log_Steam, Error, TEXT("[게임세션 오류] 스팀게임세션 끊김발생"));
	LeaveGameSession();
}

void UPTWSteamSessionSubsystem::OpenServerLevel(FName MapName, FPTWSessionConfig SessionConfig) const
{
	FString Options;
	if (!IsRunningDedicatedServer())
	{
		Options += FString::Printf(TEXT("?listen"));
	}

	if (IsRunningDedicatedServer())
	{
		GetWorld()->ServerTravel(MapName.ToString() + Options);
	}
	else
	{
		UGameplayStatics::OpenLevel(this, MapName, true, Options);
	}
}

#undef LOCTEXT_NAMESPACE
