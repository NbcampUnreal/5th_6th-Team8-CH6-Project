// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGameLiftClientSubsystem.h"
#include "Server/GameplayServerTags.h"
#include "Server/PTWAPIData.h"
#include "HttpModule.h"
#include "PTWSteamSessionSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Interfaces/IHttpResponse.h"
#include "Online/OnlineSessionNames.h"
#include "Session/PTWSessionConfig.h"
#include "UObject/Object.h"

DEFINE_LOG_CATEGORY(GameLift);
DEFINE_LOG_CATEGORY(DynamoDB);

#define LOCTEXT_NAMESPACE "GAMELIFTCLIENTSUBSYSTEM"

UPTWGameLiftClientSubsystem::UPTWGameLiftClientSubsystem()
{
	if (!IsValid(ClientAPIData))
	{
		static ConstructorHelpers::FObjectFinder<UPTWAPIData> DataAssetFinder(TEXT("/Game/_PTW/System/Server/DA_PTW_GameLift_ClientAPI.DA_PTW_GameLift_ClientAPI"));
		if (DataAssetFinder.Succeeded())
		{
			ClientAPIData = DataAssetFinder.Object; 
		}
	}
}

UPTWGameLiftClientSubsystem* UPTWGameLiftClientSubsystem::Get(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (IsValid(World))
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			return GI->GetSubsystem<UPTWGameLiftClientSubsystem>();
		}
	}
	return nullptr;
}

void UPTWGameLiftClientSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UPTWGameLiftClientSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

FString UPTWGameLiftClientSubsystem::SerializeJsonContent(const TMap<FString, FString>& Parameters)
{
	TSharedPtr<FJsonObject> ContentJsonObject = MakeShareable(new FJsonObject());
	
	// Lambda 함수와 동일한 필드로 설정.
	// ContentJsonObject->SetStringField(TEXT("playerId"), PlayerId);
	// ContentJsonObject->SetStringField(TEXT("gameSessionId"), GameSessionId);
	for (const auto& Param : Parameters)
	{
		ContentJsonObject->SetStringField(Param.Key, Param.Value);
	}
	
	FString Content;
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&Content);
	FJsonSerializer::Serialize(ContentJsonObject.ToSharedRef(), JsonWriter);
	
	return Content;
}

TMap<FString, FString> UPTWGameLiftClientSubsystem::ExtractJsonFields(const FString& JsonString, const TArray<FString>& TargetFields)
{
	TMap<FString, FString> ResultMap;

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
    
	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		const TSharedPtr<FJsonObject>* DataObjPtr = nullptr;

		if (JsonObject->TryGetObjectField(TEXT("data"), DataObjPtr) && DataObjPtr != nullptr && (*DataObjPtr).IsValid())
		{
			for (const FString& FieldName : TargetFields)
			{
				TSharedPtr<FJsonValue> JsonValue = (*DataObjPtr)->TryGetField(FieldName);
          
				if (JsonValue.IsValid() && !JsonValue->IsNull())
				{
					ResultMap.Add(FieldName, JsonValue->AsString());
				}
				else
				{
					ResultMap.Add(FieldName, TEXT("")); 
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Helper: 'data' 객체를 찾을 수 없거나 유효하지 않습니다."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Helper: JSON 파싱에 실패했습니다."));
	}

	return ResultMap;
}

void UPTWGameLiftClientSubsystem::CreateGameSession(FPTWSessionConfig& SessionConfig)
{
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::CreateGameSession_Response);
	
	const FString APIUrl = ClientAPIData->GetAPIEndPoint(GameplayServerTags::GameSessionsAPI::CreateGameSession);
	
	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	FString MaxRounds = TEXT("Long");
	if (SessionConfig.MaxRounds == GetMaxRoundsByLimit(EPTWRoundLimit::Short))
	{
		MaxRounds = TEXT("Short");
	}
	
	TMap<FString, FString> Params = {
		{ TEXT("name"),							SessionConfig.ServerName },
		{ TEXT("maximumPlayerSessionCount"),	FString::FromInt(SessionConfig.MaxPlayers) },
		{ TEXT("maxRoundType"),					MaxRounds }, 
	};
	
	GetWorld()->GetTimerManager().SetTimer(CheckSessionLitmitTimer, 20.0f, false);
	const FString Content = SerializeJsonContent(Params);
	Request->SetContentAsString(Content);
	if (Request->ProcessRequest())
	{
		UE_LOG(GameLift, Display, TEXT("[게임세션 생성요청] 게임리프트 게임세션 생성요청 전송 완료"));
	}
	else
	{
		if (OnGameLiftSessionMessageReceived.IsBound())
		{
			FText ErrorMessage = LOCTEXT("HTTPRequestFailed", "네트워크 문제로 인해 서버에 세션 생성 요청을 보내지 못했습니다. 연결 상태를 확인해 주세요.");
			OnGameLiftSessionMessageReceived.Broadcast(ErrorMessage);
		}
		UE_LOG(GameLift, Error, TEXT("[게임세션 생성요청] 게임리프트 게임세션 생성요청 전송 실패"));
		GetWorld()->GetTimerManager().ClearTimer(CheckSessionLitmitTimer);
	}
}

void UPTWGameLiftClientSubsystem::CreateGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid() || Response->GetResponseCode() != 200)
	{
		if (OnGameLiftSessionMessageReceived.IsBound())
		{
			FText ErrorMessage = LOCTEXT("SessionCreateFailed", "알 수 없는 오류가 발생해 세션 생성에 실패했습니다.");
			OnGameLiftSessionMessageReceived.Broadcast(ErrorMessage);
		}
		UE_LOG(GameLift, Error, TEXT("[게임세션 생성응답] 게임리프트 게임세션 생성 실패 응답 (Code:%d)"), Response->GetResponseCode());
		return;
	}
	FString GameSessionIdName = TEXT("GameSessionId");
	TArray<FString> FieldsToExtract = { GameSessionIdName };
	
	TMap<FString, FString> SessionData = ExtractJsonFields(Response->GetContentAsString(), FieldsToExtract);
	
	if (SessionData.Contains(GameSessionIdName))
	{
		FString GameSessionId = SessionData[GameSessionIdName];
		CheckSessionStatus(GameSessionId, true);
	}
}

void UPTWGameLiftClientSubsystem::CheckSessionStatus(const FString& SessionId, bool bIsLoop)
{
	check(ClientAPIData);

	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	
	if (bIsLoop)
	{
		Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::CheckSessionStatusLoop_Response, SessionId);
	}
	else
	{
		Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::CheckSessionStatus_Response);
	}
	
	const FString APIUrl = ClientAPIData->GetAPIEndPoint(GameplayServerTags::GameSessionsAPI::CheckSessionStatus);
	FString RefinedURL = APIUrl;
	
	RefinedURL += TEXT("?gameSessionId=") + FGenericPlatformHttp::UrlEncode(SessionId);
	
	Request->SetURL(RefinedURL);
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	if (Request->ProcessRequest())
	{
		UE_LOG(DynamoDB, Display, TEXT("[DB속성 확인요청] DB:GameSessionLists ServerState 확인요청 전송성공 (GameSessionId:%s"), *SessionId);
	}
	else
	{
		if (OnGameLiftSessionMessageReceived.IsBound())
		{
			FText ErrorMessage = LOCTEXT("HTTPRequestFailed", "네트워크 문제로 인해 서버에 세션 생성 요청을 보내지 못했습니다. 연결 상태를 확인해 주세요.");
			OnGameLiftSessionMessageReceived.Broadcast(ErrorMessage);
		}
		UE_LOG(DynamoDB, Error, TEXT("[DB속성 확인요청] DB:GameSessionLists ServerState 확인요청 전송실패 (GameSessionId:%s"), *SessionId);
		GetWorld()->GetTimerManager().ClearTimer(CheckSessionLitmitTimer);
	}
}

void UPTWGameLiftClientSubsystem::CheckSessionStatus_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid() || Response->GetResponseCode() != 200)
	{
		if (OnGameLiftSessionMessageReceived.IsBound())
		{
			FText ErrorMessage = LOCTEXT("SessionCheckFailed", "알 수 없는 오류가 발생해 세션 체크에 실패하였습니다.");
			OnGameLiftSessionMessageReceived.Broadcast(ErrorMessage);
		}
		UE_LOG(DynamoDB, Error, TEXT("[DB속성 확인응답] DB:GameSessionLists ServerState 확인 실패"));
	}
}

void UPTWGameLiftClientSubsystem::CheckSessionStatusLoop_Response(FHttpRequestPtr Request, 
	FHttpResponsePtr Response, bool bWasSuccessful, FString SessionId)
{
	if (!bWasSuccessful || !Response.IsValid() || (Response->GetResponseCode() != 202 && Response->GetResponseCode() != 200))
	{
		GetWorld()->GetTimerManager().ClearTimer(CheckSessionLitmitTimer);
		if (OnGameLiftSessionMessageReceived.IsBound())
		{
			FText ErrorMessage = LOCTEXT("SessionCheckFailed", "알 수 없는 오류가 발생해 세션 체크에 실패하였습니다.");
			OnGameLiftSessionMessageReceived.Broadcast(ErrorMessage);
		}
		UE_LOG(DynamoDB, Error, TEXT("[DB속성 확인응답] DB:GameSessionLists ServerState 확인실패"));
	}
	else if (bWasSuccessful && Response.IsValid())
	{
		UE_LOG(DynamoDB, Error, TEXT("[DB속성 확인응답] DB:GameSessionLists ServerState 확인성공"));
		if (Response->GetResponseCode() == 202)
		{
			UE_LOG(DynamoDB, Display, TEXT("[DB속성 확인응답] DB:GameSessionLists ServerState: ACTIVATING (GameSessionId:%s"), *SessionId);
			WaitForSessionActivation(SessionId);
		}
		else if (Response->GetResponseCode() == 200)
		{
			GetWorld()->GetTimerManager().ClearTimer(CheckSessionLitmitTimer);
			UE_LOG(DynamoDB, Display, TEXT("[DB속성 확인응답] DB:GameSessionLists ServerState: ACTIVE (GameSessionId:%s"), *SessionId);
			CreatePlayerSession(GetUniquePlayerId(), SessionId);
		}
	}
}

void UPTWGameLiftClientSubsystem::WaitForSessionActivation(const FString& SessionId)
{
	if (APlayerController* PC = GEngine->GetFirstLocalPlayerController(GetWorld()))
	{
		if (GetWorld()->GetTimerManager().IsTimerActive(CheckSessionLitmitTimer))
		{
			FTimerHandle CreateSessionTimer;
			PC->GetWorldTimerManager().SetTimer(CreateSessionTimer,[this, SessionId]()
			{
				CheckSessionStatus(SessionId, true);
				UE_LOG(LogTemp, Display, TEXT("CheckSessionStatus 재시도"));
			}, 2.0f, false);
		}
		else
		{
			if (OnGameLiftSessionMessageReceived.IsBound())
			{
				FText ErrorMessage = LOCTEXT("SessionCheckTimeOut", "세션 타임아웃이 발생했습니다.");
				OnGameLiftSessionMessageReceived.Broadcast(ErrorMessage);
				UE_LOG(LogTemp, Warning, TEXT("WaitForSessionActivation 타임아웃"));
			}
		}
	}
}

void UPTWGameLiftClientSubsystem::DescribeGameSession(const FString& SessionId)
{
	if (!IsValid(ClientAPIData))
	{
		UE_LOG(LogTemp, Error, TEXT("DescribeGameSession Failed: APIData is NULL!"));
		return;
	}

	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::DescribeGameSession_Response);
	
	const FString APIUrl = ClientAPIData->GetAPIEndPoint(GameplayServerTags::GameSessionsAPI::DescribeGameSession);
	FString RefinedURL = APIUrl;
	
	RefinedURL += TEXT("?gameSessionId=") + FGenericPlatformHttp::UrlEncode(SessionId);
	
	Request->SetURL(RefinedURL);
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->ProcessRequest();
}

void UPTWGameLiftClientSubsystem::DescribeGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid() || Response->GetResponseCode() != 200)
	{
		UE_LOG(LogTemp, Error, TEXT("DescribeGameSession 통신 실패."));
		return;
	}
	
	FPTWGameLiftGameSession GameSession;
	if (ParseDataFromJson<FPTWGameLiftGameSession>(Response->GetContentAsString(), GameSession))
	{
		const FString GameSessionId = GameSession.GameSessionId;
		const FString GameSessionStatus = GameSession.Status;
		// TryJoinGameSession(GameSessionStatus, GameSessionId);
	}
}

void UPTWGameLiftClientSubsystem::CreatePlayerSession(const FString& PlayerId, const FString& GameSessionId)
{
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::CreatePlayerSession_Response);
	
	const FString APIUrl = ClientAPIData->GetAPIEndPoint(GameplayServerTags::GameSessionsAPI::CreatePlayerSession);
	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	
	TMap<FString, FString> Params = {
		{ TEXT("playerId"), PlayerId },
		{ TEXT("gameSessionId"), GameSessionId }
	};
	
	const FString Content = SerializeJsonContent(Params);
	
	Request->SetContentAsString(Content);
	if (Request->ProcessRequest())
	{
		UE_LOG(GameLift, Display, TEXT("[플레이어세션 생성요청] 게임리프트 게임세션 - 플레이어세션 생성요청 전송완료 (GameSessionId:%s)"), *GameSessionId);
	}
	else
	{
		if (OnGameLiftSessionMessageReceived.IsBound())
		{
			FText ErrorMessage = LOCTEXT("HTTPRequestFailed", "네트워크 문제로 인해 서버에 세션 생성 요청을 보내지 못했습니다. 연결 상태를 확인해 주세요.");
			OnGameLiftSessionMessageReceived.Broadcast(ErrorMessage);
		}
		UE_LOG(GameLift, Error, TEXT("[플레이어세션 생성요청] 게임리프트 게임세션 - 플레이어세션 생성요청 전송실패 (GameSessionId:%s)"), *GameSessionId);
		GetWorld()->GetTimerManager().ClearTimer(CheckSessionLitmitTimer);
	}
}

void UPTWGameLiftClientSubsystem::CreatePlayerSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("Create PlayerSession Response Received"));
	
	if (!bWasSuccessful || !Response.IsValid() || Response->GetResponseCode() != 200)
	{
		if (OnGameLiftSessionMessageReceived.IsBound())
		{
			FText ErrorMessage = LOCTEXT("SessionCheckTimeOut", "세션 참여에 실패하였습니다.");
			OnGameLiftSessionMessageReceived.Broadcast(ErrorMessage);
		}
		UE_LOG(GameLift, Error, TEXT("[플레이어세션 생성응답] 게임리프트 게임세션 - 플레이어세션 생성실패 응답 (Code:%d)"), Response->GetResponseCode());
		return;
	}
	UE_LOG(GameLift, Display, TEXT("[플레이어세션 생성응답] 게임리프트 게임세션 - 플레이어세션 생성성공 응답 (Code:%d)"), Response->GetResponseCode());
	
	FString PlayerSessionIdName = TEXT("PlayerSessionId");
	FString PortName = TEXT("Port");
	FString SteamIdName = TEXT("SteamId");
	
	TArray<FString> FieldsToExtract = { PlayerSessionIdName, PortName, SteamIdName };
	TMap<FString, FString> SessionData = ExtractJsonFields(Response->GetContentAsString(), FieldsToExtract);
	if (SessionData.Contains(PlayerSessionIdName) && SessionData.Contains(PortName) && SessionData.Contains(SteamIdName))
	{
		// ("steam.%s:%s?PlayerSessionId=%s")
		
		FString ConnectURL = FString::Printf(TEXT("?PlayerSessionId=%s"), *SessionData[PlayerSessionIdName]);
		UE_LOG(LogTemp, Warning, TEXT("%s"), *ConnectURL);
		
		FindByIdAndJoinSession(SessionData[SteamIdName], ConnectURL);
	}
}

void UPTWGameLiftClientSubsystem::SearchGameSessions()
{
	FHttpModule* Http = &FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
	
	Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::SearchGameSessions_Response);
	
	const FString APIUrl = ClientAPIData->GetAPIEndPoint(GameplayServerTags::GameSessionsAPI::SearchGameSessions);
	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->ProcessRequest();
	UE_LOG(LogTemp, Log, TEXT("Searching for game sessions..."));
}

void UPTWGameLiftClientSubsystem::SearchGameSessions_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid() || Response->GetResponseCode() != 200)
	{
		UE_LOG(LogTemp, Error, TEXT("HTTP Request failed!"));
		return;
	}
	
	TArray<FPTWGameSessionListsTable> GameSessionLists;
	ParseDataArrayFromJson(Response->GetContentAsString(), GameSessionLists);
	if (GameSessionLists.IsEmpty()) return;
	
	if (OnSessionSearchComplete.IsBound())
	{
		OnSessionSearchComplete.Broadcast(GameSessionLists);
	}
}

void UPTWGameLiftClientSubsystem::SearchQuickSession()
{
	FHttpModule* Http = &FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "SearchQuickSession");
	Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::SearchQuickSession_Response);
	
	const FString APIUrl = ClientAPIData->GetAPIEndPoint(GameplayServerTags::GameSessionsAPI::SearchQuickSession);
	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	
	GetWorld()->GetTimerManager().SetTimer(CheckSessionLitmitTimer, 20.f, false);
	if (Request->ProcessRequest())
	{
		UE_LOG(DynamoDB, Display, TEXT("[DB레코드 추출요청] DB:GameSessionLists 레코드 추출요청 전송성공"));
	}
	else
	{
		if (OnGameLiftSessionMessageReceived.IsBound())
		{
			FText ErrorMessage = LOCTEXT("HTTPRequestFailed", "네트워크 문제로 인해 서버에 세션 생성 요청을 보내지 못했습니다. 연결 상태를 확인해 주세요.");
			OnGameLiftSessionMessageReceived.Broadcast(ErrorMessage);
		}
		UE_LOG(DynamoDB, Error, TEXT("[DB레코드 추출요청] DB:GameSessionLists 레코드 추출요청 전송실패"));
		GetWorld()->GetTimerManager().ClearTimer(CheckSessionLitmitTimer);
	}
}

void UPTWGameLiftClientSubsystem::SearchQuickSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid() || Response->GetResponseCode() != 200)
	{
		GetWorld()->GetTimerManager().ClearTimer(CheckSessionLitmitTimer);
		if (OnGameLiftSessionMessageReceived.IsBound())
		{
			FText ErrorMessage = LOCTEXT("SessionCheckTimeOut", "세션 참여에 실패하였습니다.");
			OnGameLiftSessionMessageReceived.Broadcast(ErrorMessage);
		}
		UE_LOG(DynamoDB, Error, TEXT("[DB레코드 추출응답] DB:GameSessionLists 레코드 추출실패 응답 (Code:%d)"), Response->GetResponseCode());
		return;
	}
	UE_LOG(DynamoDB, Display, TEXT("[DB레코드 추출응답] DB:GameSessionLists 레코드 추출성공 응답 (Code:%d)"), Response->GetResponseCode());
	
	TArray<FPTWGameSessionListsTable> GameSessionList;
	ParseDataArrayFromJson(Response->GetContentAsString(), GameSessionList);
	
	TArray<FPTWGameSessionListsTable> ActiveList;
	TArray<FPTWGameSessionListsTable> ActivatingList;
	
	for (FPTWGameSessionListsTable& TargetGameSession : GameSessionList)
	{
		if (TargetGameSession.ServerState == TEXT("ACTIVE"))
		{
			ActiveList.Add(TargetGameSession);
		}
		else if (TargetGameSession.ServerState == TEXT("ACTIVATING"))
		{
			ActivatingList.Add(TargetGameSession);
		}
	}
	
	UE_LOG(DynamoDB, Display, TEXT("[DB레코드 추출응답] DB:GameSessionLists ServerState:ACTIVE 서버:%d"), ActiveList.Num());
	UE_LOG(DynamoDB, Display, TEXT("[DB레코드 추출응답] DB:GameSessionLists ServerState:ACTIVATING 서버:%d"), ActivatingList.Num());
	
	if (!ActiveList.IsEmpty())
	{
		UE_LOG(LogTemp, Display, TEXT("ServerState:ACTIVE 서버접속 (GameSessionId:%s)"), *ActiveList[0].GameSessionId);
		CreatePlayerSession(GetUniquePlayerId(), ActiveList[0].GameSessionId);
	}
	else if (!ActivatingList.IsEmpty())
	{
		UE_LOG(LogTemp, Display, TEXT("ServerState:ACTIVATING 서버접속 (GameSessionId:%s)"), *ActivatingList[0].GameSessionId);
		CheckSessionStatus(ActivatingList[0].GameSessionId, true);
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("유효한 서버 비존재. 직접서버 생성"));
		FPTWSessionConfig SessionConfig;
		SessionConfig.ServerName = "QuickMatchGameSessions";
		SessionConfig.MaxPlayers = 8;
		SessionConfig.MaxRounds = GetMaxRoundsByLimit(EPTWRoundLimit::Short);
		CreateGameSession(SessionConfig);
	}
}

void UPTWGameLiftClientSubsystem::FindByIdAndJoinSession(const FString& SteamId, const FString& Options)
{
	UPTWSteamSessionSubsystem* SteamSessionSubsystem = UPTWSteamSessionSubsystem::Get(this);
	IOnlineSessionPtr SessionInterface = SteamSessionSubsystem->GetSessionInterface();
	
	if(!SessionInterface.IsValid()) return;
	SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);

	TSharedPtr<FOnlineSessionSearch> DedicatedSessionSearch = MakeShareable(new FOnlineSessionSearch());
	DedicatedSessionSearch->bIsLanQuery = false;
	DedicatedSessionSearch->MaxSearchResults = 100;
	DedicatedSessionSearch->QuerySettings.Set(SEARCH_DEDICATED_ONLY, true, EOnlineComparisonOp::Equals);
	DedicatedSessionSearch->QuerySettings.Set(SEARCH_LOBBIES, false, EOnlineComparisonOp::Equals);

	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
		FOnFindSessionsCompleteDelegate::CreateWeakLambda(this, [=, this](bool bWasSuccessful)
		{
			if(SessionInterface.IsValid())
			{
				SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
			}
			
			if (bWasSuccessful && DedicatedSessionSearch.IsValid())
			{
				UE_LOG(LogTemp, Log, TEXT("Search Complete! Found %d sessions."), DedicatedSessionSearch->SearchResults.Num());
				for (const FOnlineSessionSearchResult& SearchResult : DedicatedSessionSearch->SearchResults)
				{
					FString TargetSessionId;
					SearchResult.Session.SessionSettings.Get(PTWSessionKey::SteamId, TargetSessionId);

					if (TargetSessionId.IsEmpty()) continue;

					if (SteamId == TargetSessionId)
					{
						SteamSessionSubsystem->JoinGameSession(FOnlineSessionSearchResultBP(SearchResult), Options);
						return;
					}
				}
			}
			// 가끔 스팀에서 세션 탐색에 실패하면 재시도
			if (GetWorld()->GetTimerManager().IsTimerActive(CheckSessionLitmitTimer))
			{
				FTimerHandle TempTimerHandle;
				GetWorld()->GetTimerManager().SetTimer(TempTimerHandle, [=, this]()
				{
					FindByIdAndJoinSession(SteamId, Options);
				}, 2.0f, false);
			}
		})
	);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), DedicatedSessionSearch.ToSharedRef()))
	{
		UE_LOG(LogTemp, Log, TEXT("Session search started..."));
	}
	else
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		UE_LOG(LogTemp, Warning, TEXT("Failed to start session search."));
		if (OnGameLiftSessionMessageReceived.IsBound())
		{
			FText ErrorMessage = LOCTEXT("FindSteamSessionFailed", "스팀 세션 탐색에 실패했습니다.");
			OnGameLiftSessionMessageReceived.Broadcast(ErrorMessage);
		}
	}
}

FString UPTWGameLiftClientSubsystem::GetUniquePlayerId() const
{
	if (APlayerController* LocalPlayerController = GEngine->GetFirstLocalPlayerController(GetWorld()))
	{
		if (APlayerState* LocalPlayerState = LocalPlayerController->GetPlayerState<APlayerState>())
		{
			if (LocalPlayerState->GetUniqueId().IsValid())
			{
				const FString UniqueId = TEXT("Player_") + FString::FromInt(LocalPlayerState->GetUniqueID());
				return UniqueId;
			}
		}
	}
	return FString();
}

#undef LOCTEXT_NAMESPACE
