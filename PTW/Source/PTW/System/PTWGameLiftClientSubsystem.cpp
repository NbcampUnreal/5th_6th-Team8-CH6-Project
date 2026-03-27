// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGameLiftClientSubsystem.h"
#include "Server/GameplayServerTags.h"
#include "Server/PTWAPIData.h"
#include "HttpModule.h"
#include "GameFramework/PlayerState.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Interfaces/IHttpResponse.h"
#include "Session/PTWSessionConfig.h"
#include "UObject/Object.h"

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

void UPTWGameLiftClientSubsystem::CreateGameSession(FPTWSessionConfig& SessionConfig)
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

void UPTWGameLiftClientSubsystem::CreateGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
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

void UPTWGameLiftClientSubsystem::CheckSessionStatus(const FString& SessionId)
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

void UPTWGameLiftClientSubsystem::CheckSessionStatus_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
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

	TryJoinGameSession(GameSessionLists.GameSessionId, GameSessionLists.SteamId, GameSessionLists.Status);
}

void UPTWGameLiftClientSubsystem::TryJoinGameSession(const FString& SessionId, const FString& SteamId, const FString& Status)
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

void UPTWGameLiftClientSubsystem::WaitForSessionActivation(const FString& SessionId)
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
	Request->ProcessRequest();
}

void UPTWGameLiftClientSubsystem::CreatePlayerSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
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
