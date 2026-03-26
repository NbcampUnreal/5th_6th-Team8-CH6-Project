// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGameLiftSubsystem.h"
#include "Server/GameplayServerTags.h"
#include "Server/PTWAPIData.h"
#include "HttpModule.h"
#include "JsonObjectConverter.h"
#include "PTWSessionSubsystem.h"
#include "Server/PTWHTTPRequestTypes.h"
#include "GameFramework/PlayerState.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Interfaces/IHttpResponse.h"
#include "Session/PTWSessionConfig.h"
#include "UObject/Object.h"

#define LOCTEXT_NAMESPACE "GAMELIFTSUBSYSTEM"
UPTWGameLiftSubsystem::UPTWGameLiftSubsystem()
{
	// TODO: 임시 하드코딩
	if (!IsValid(ClientAPIData))
	{
		static ConstructorHelpers::FObjectFinder<UPTWAPIData> DataAssetFinder(TEXT("/Game/_PTW/System/Server/DA_PTW_GameLift_ClientAPI.DA_PTW_GameLift_ClientAPI"));
		if (DataAssetFinder.Succeeded())
		{
			ClientAPIData = DataAssetFinder.Object; 
		}
	}
	
	if (!IsValid(ServerAPIData))
	{
		static ConstructorHelpers::FObjectFinder<UPTWAPIData> DataAssetFinder(TEXT("/Game/_PTW/System/Server/DA_PTW_GameLift_ServerAPI.DA_PTW_GameLift_ServerAPI"));
		if (DataAssetFinder.Succeeded())
		{
			ServerAPIData = DataAssetFinder.Object; 
		}
	}
}

void UPTWGameLiftSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
#if WITH_GAMELIFT
	MapLoadDelegateHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::OnMapLoaded);
#endif
	
}

void UPTWGameLiftSubsystem::Deinitialize()
{
	if (MapLoadDelegateHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(MapLoadDelegateHandle);
		MapLoadDelegateHandle.Reset();
	}
	Super::Deinitialize();
}

FString UPTWGameLiftSubsystem::SerializeJsonContent(const TMap<FString, FString>& Parameters)
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

void UPTWGameLiftSubsystem::RequestListFleets()
{
	check(ClientAPIData);
	
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	
	Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::ListFleets_Response);
	
	const FString APIUrl = ClientAPIData->GetAPIEndPoint(GameplayServerTags::GameSessionsAPI::ListFleets);
	
	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->ProcessRequest();
	
	UE_LOG(LogTemp, Warning, TEXT("ListFleets_Response Made"));
}

void UPTWGameLiftSubsystem::CreateGameSession(FPTWSessionConfig& SessionConfig)
{
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::CreateGameSession_Response);
	
	const FString APIUrl = ClientAPIData->GetAPIEndPoint(GameplayServerTags::GameSessionsAPI::CreateGameSession);
	
	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	
	TMap<FString, FString> Params = {
		{ TEXT("name"),							SessionConfig.ServerName },
		{ TEXT("maximumPlayerSessionCount"),	FString::FromInt(SessionConfig.MaxPlayers) }
	};
	
	const FString Content = SerializeJsonContent(Params);
	Request->SetContentAsString(Content);
	Request->ProcessRequest();
	
	GetWorld()->GetTimerManager().SetTimer(CheckSessionLitmitTimer, 20.0f, false);
}

void UPTWGameLiftSubsystem::CreateGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Create Game Session Response Received");
	if (!bWasSuccessful || !Response.IsValid() || Response->GetResponseCode() != 200)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateGameSession Failed."));
		if (OnGameLiftSessionMessageReceived.IsBound())
		{
			FText ErrorMessage = LOCTEXT("SessionCreateFailed", "알 수 없는 오류가 발생해 세션 생성에 실패했습니다.");
			OnGameLiftSessionMessageReceived.Broadcast(ErrorMessage);
		}
		return;
	}
	
	FPTWGameLiftGameSession GameSession;
	if (ParseDataFromJson<FPTWGameLiftGameSession>(Response->GetContentAsString(), GameSession))
	{
		const FString GameSessionId = GameSession.GameSessionId;
		CheckSessionStatus(GameSessionId);
	}
}

void UPTWGameLiftSubsystem::CheckSessionStatus(const FString& SessionId)
{
	if (!IsValid(ClientAPIData))
	{
		UE_LOG(LogTemp, Error, TEXT("DescribeGameSession Failed: APIData is NULL!"));
		return;
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, "CheckSessionStatus");
	
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::CheckSessionStatus_Response);
	
	const FString APIUrl = ClientAPIData->GetAPIEndPoint(GameplayServerTags::GameSessionsAPI::CheckSessionStatus);
	FString RefinedURL = APIUrl;
	
	RefinedURL += TEXT("?gameSessionId=") + FGenericPlatformHttp::UrlEncode(SessionId);
	
	Request->SetURL(RefinedURL);
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->ProcessRequest();
}

void UPTWGameLiftSubsystem::CheckSessionStatus_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, "CheckSessionStatus_Response");
	
	if (!bWasSuccessful || !Response.IsValid() || Response->GetResponseCode() != 200)
	{
		UE_LOG(LogTemp, Error, TEXT("CheckSessionStatus 통신 실패."));
		GetWorld()->GetTimerManager().ClearTimer(CheckSessionLitmitTimer);
		if (OnGameLiftSessionMessageReceived.IsBound())
		{
			FText ErrorMessage = LOCTEXT("SessionCheckFailed", "알 수 없는 오류가 발생해 세션 체크에 실패하였습니다.");
			OnGameLiftSessionMessageReceived.Broadcast(ErrorMessage);
		}
		return;
	}
	
	FGameSessionListsTable GameSessionLists;
	if (ParseDataFromJson<FGameSessionListsTable>(Response->GetContentAsString(), GameSessionLists))
	{
		TryJoinGameSession(GameSessionLists.GameSessionId, GameSessionLists.SteamId, GameSessionLists.Status);
	}
}

void UPTWGameLiftSubsystem::TryJoinGameSession(const FString& SessionId, const FString& SteamId, const FString& Status)
{
	if (Status.Equals(TEXT("ACTIVE")))
	{
		UE_LOG(LogTemp, Error, TEXT("Found active Game Session. Creating a Player Session..."));
		GetWorld()->GetTimerManager().ClearTimer(CheckSessionLitmitTimer);
		CreatePlayerSession(GetUniquePlayerId(), SessionId);
	}
	else if (Status.Equals(TEXT("ACTIVATING")))
	{
		WaitForSessionActivation(SessionId);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Unknown Status %s"), *Status);
	}
}

void UPTWGameLiftSubsystem::WaitForSessionActivation(const FString& SessionId)
{
	if (APlayerController* PC = GEngine->GetFirstLocalPlayerController(GetWorld()))
	{
		if (GetWorld()->GetTimerManager().IsTimerActive(CheckSessionLitmitTimer))
		{
			FTimerHandle CreateSessionTimer;
			PC->GetWorldTimerManager().SetTimer(CreateSessionTimer,
		   [this, SessionId]()
		   {
			   CheckSessionStatus(SessionId);
		   }, 2.0f, false);
		}
		else
		{
			if (OnGameLiftSessionMessageReceived.IsBound())
			{
				FText ErrorMessage = LOCTEXT("SessionCheckTimeOut", "세션 타임아웃이 발생했습니다.");
				OnGameLiftSessionMessageReceived.Broadcast(ErrorMessage);
			}
		}
	}
}

void UPTWGameLiftSubsystem::DescribeGameSession(const FString& SessionId)
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

void UPTWGameLiftSubsystem::DescribeGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
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

void UPTWGameLiftSubsystem::CreatePlayerSession(const FString& PlayerId, const FString& GameSessionId)
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
	Request->ProcessRequest();
}

void UPTWGameLiftSubsystem::CreatePlayerSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("Create PlayerSession Response Received"));
	
	if (!bWasSuccessful || !Response.IsValid() || Response->GetResponseCode() != 200)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "CreatePlayerSession 통신 실패.");
		if (OnGameLiftSessionMessageReceived.IsBound())
		{
			FText ErrorMessage = LOCTEXT("SessionCheckTimeOut", "세션 참여에 실패하였습니다.");
			OnGameLiftSessionMessageReceived.Broadcast(ErrorMessage);
		}
		return;
	}
	
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "CreatePlayerSession_Response");
	
	FPTWGameLiftPlayerSession PlayerSession;
	if (ParseDataFromJson<FPTWGameLiftPlayerSession>(Response->GetContentAsString(), PlayerSession))
	{
		FString ConnectURL = FString::Printf(TEXT("steam.%s:%d?PlayerSessionId=%s"),
			  *PlayerSession.IpAddress,			  // 서버스팀 ID
			  PlayerSession.Port,
			  *PlayerSession.PlayerSessionId      // GameLift 세션 인증용 ID
		  );
		UE_LOG(LogTemp, Warning, TEXT("%s"), *ConnectURL);
		// const FString IpAndPort = PlayerSession.IpAddress; + TEXT(":") + FString::FromInt(PlayerSession.Port);
		// const FName Address = FName(*IpAndPort);
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, "ClientTravel");
			PC->ClientTravel(ConnectURL, TRAVEL_Absolute, false);
		}
	}
}

void UPTWGameLiftSubsystem::SearchGameSessions()
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

void UPTWGameLiftSubsystem::SearchGameSessions_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("HTTP Request failed!"));
		return;
	}
	
	TArray<FPTWGameLiftGameSession> GameSessions;
	ParseDataArrayFromJson(Response->GetContentAsString(), GameSessions);
	if (GameSessions.IsEmpty()) return;
	
	if (OnSessionSearchComplete.IsBound())
	{
		OnSessionSearchComplete.Broadcast(GameSessions);
	}
}

bool UPTWGameLiftSubsystem::ContainErrors(TSharedPtr<FJsonObject> JsonObject)
{
	if (JsonObject->HasField(TEXT("errorType")) || JsonObject->HasField(TEXT("errorMessage")))
	{
		FString ErrorType = JsonObject->HasField(TEXT("errorType")) ? JsonObject->GetStringField(TEXT("errorType")) : TEXT("UnKnown Error");
		FString ErrorMessage = JsonObject->HasField(TEXT("errorType")) ? JsonObject->GetStringField(TEXT("errorType")) : TEXT("UnKnown Error Message");
		
		UE_LOG(LogTemp, Error, TEXT("Error Type: %s"), *ErrorType);
		UE_LOG(LogTemp, Warning, TEXT("Error Message: %s"), *ErrorMessage);
		
		return true;
	}
	if (JsonObject->HasField(TEXT("$fault")))
	{
		FString ErrorType = JsonObject->HasField(TEXT("name")) ? JsonObject->GetStringField(TEXT("name")) : TEXT("UnKnown Error");
		UE_LOG(LogTemp, Error, TEXT("Error Type: %s"), *ErrorType);
		
		return true;
	}
	return false;
}

void UPTWGameLiftSubsystem::DumpMetadata(TSharedPtr<FJsonObject> JsonObject)
{
	if(JsonObject->HasField(TEXT("$metadata")))
	{
		TSharedPtr<FJsonObject> MetaDataJsonObject = JsonObject->GetObjectField(TEXT("$metadata"));
		FPTWMetaData PTWMetaData;
		FJsonObjectConverter::JsonObjectToUStruct(MetaDataJsonObject.ToSharedRef(), &PTWMetaData);
			
		PTWMetaData.Dump();
	}
}

FString UPTWGameLiftSubsystem::GetUniquePlayerId() const
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

void UPTWGameLiftSubsystem::ListFleets_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("ListFleets_Response Received"));
	
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		if (ContainErrors(JsonObject))
		{
			// OnListFleetsResponseReceived.Broadcast(FPTWListFleetsResponse(), false);
			return;
		}
		
		DumpMetadata(JsonObject);
		
		FPTWListFleetsResponse ListFleetsResponse;
		FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), &ListFleetsResponse);
		ListFleetsResponse.Dump();
		
		// OnListFleetsResponseReceived.Broadcast(ListFleetsResponse, true);
	}
}

#if WITH_GAMELIFT
void UPTWGameLiftSubsystem::OnMapLoaded(UWorld* LoadedWorld)
{
	if (!LoadedWorld) return;
	if (!IsRunningDedicatedServer()) return;
	
	if (GameLiftSdkModule)
	{
		GameLiftSdkModule->ActivateGameSession();
		ReportServerInfoToBackend();
		// GameLiftSdkModule = nullptr;
	}
	
	if (MapLoadDelegateHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(MapLoadDelegateHandle);
		MapLoadDelegateHandle.Reset();
	}
}

void UPTWGameLiftSubsystem::ReportServerInfoToBackend()
{
	// FString GameSessionId = FString(InGameSession.GetGameSessionId());
	FString GameSessionId = GameLiftSdkModule->GetGameSessionId().GetResult();
	FString SteamId;
	
	if (UWorld* World = GetWorld())
	{
		if(UGameInstance* GI = World->GetGameInstance())
		{
			if (UPTWSessionSubsystem* SessionSubsystem = GI->GetSubsystem<UPTWSessionSubsystem>())
			{
				SteamId = SessionSubsystem->GetSteamServerID();
			}
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("GameSessionId: %s"), *GameSessionId);
	UE_LOG(LogTemp, Warning, TEXT("SteamId: %s"), *SteamId);
	
	if (!SteamId.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("ReportServerInfoToBackend SteamId Is Valid"));
		TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
		Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::ReportServerInfoToBackend_Response);
		
		const FString APIUrl = ServerAPIData->GetAPIEndPoint(GameplayServerTags::GameSessionsAPI::ReportServerInfoToBackend);
		
		Request->SetURL(APIUrl);
		Request->SetVerb(TEXT("POST"));
		Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
		
		TMap<FString, FString> Params = {
			{ TEXT("gameSessionId"),	GameSessionId },
			{ TEXT("steamId"),			SteamId }
		};
		
		const FString Content = SerializeJsonContent(Params);
		Request->SetContentAsString(Content);
		Request->ProcessRequest();
	}
	else
	{
		// NotSteamId
	}
}

void UPTWGameLiftSubsystem::ReportServerInfoToBackend_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		UE_LOG(LogTemp, Log, TEXT("ReportServerInfoToBackend_Response Successful"));
		UE_LOG(LogTemp, Log, TEXT("Successfully reported Steam ID to Backend."));
		FString GameSessionId = GameLiftSdkModule->GetGameSessionId().GetResult();
		if (!GameSessionId.IsEmpty())
		{
			if (UWorld* World = GetWorld())
			{
				if(UGameInstance* GI = World->GetGameInstance())
				{
					if (UPTWSessionSubsystem* SessionSubsystem = GI->GetSubsystem<UPTWSessionSubsystem>())
					{
					
						SessionSubsystem->OnGameSessionActivated(GameSessionId);
					}
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("ReportServerInfoToBackend_Response Failed"));
		UE_LOG(LogTemp, Error, TEXT("Failed to report Steam ID to Backend."));
	}
}

void UPTWGameLiftSubsystem::SetupMapLoadDelegateHandle()
{
	if (!IsRunningDedicatedServer()) return;
	if (MapLoadDelegateHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(MapLoadDelegateHandle);
		MapLoadDelegateHandle.Reset();
	}
	MapLoadDelegateHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::OnMapLoaded);
}

void UPTWGameLiftSubsystem::RemovePlayerSession(FString PlayerSessionId)
{
	if (GameLiftSdkModule)
	{
		GameLiftSdkModule->RemovePlayerSession(PlayerSessionId);
		UE_LOG(LogTemp, Log, TEXT("GameLift 플레이어 세션 제거 완료: %s"), *PlayerSessionId);
	}
}

void UPTWGameLiftSubsystem::ExitGameSession()
{
	if (GameLiftSdkModule)
	{
		GameLiftSdkModule->ProcessEnding();
	}
}

#endif
#undef LOCTEXT_NAMESPACE
