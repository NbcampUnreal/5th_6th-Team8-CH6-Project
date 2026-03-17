// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGameLiftSubsystem.h"
#include "Server/GameplayServerTags.h"
#include "Server/PTWAPIData.h"
#include "HttpModule.h"
#include "JsonObjectConverter.h"
#include "Server/PTWHTTPRequestTypes.h"
#include "GameFramework/PlayerState.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Interfaces/IHttpResponse.h"
#include "Kismet/GameplayStatics.h"
#include "Session/PTWSessionConfig.h"
#include "UObject/Object.h"

UPTWGameLiftSubsystem::UPTWGameLiftSubsystem()
{
	// TODO: 임시 하드코딩
	static ConstructorHelpers::FObjectFinder<UPTWAPIData> DataAssetFinder(TEXT("/Game/_PTW/System/Server/DA_GameSessionAPIData.DA_GameSessionAPIData"));
	if (DataAssetFinder.Succeeded())
	{
		APIData = DataAssetFinder.Object; 
	}
}

void UPTWGameLiftSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	MapLoadDelegateHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::OnMapLoaded);
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

void UPTWGameLiftSubsystem::OnMapLoaded(UWorld* LoadedWorld)
{
	if (!LoadedWorld) return;
#if WITH_GAMELIFT && UE_SERVER
	if (GameLiftSdkModule)
	{
		GameLiftSdkModule->ActivateGameSession();
	}
#endif
}

void UPTWGameLiftSubsystem::SetupMapLoadDelegateHandle()
{
	if (MapLoadDelegateHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(MapLoadDelegateHandle);
	}
	MapLoadDelegateHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::OnMapLoaded);
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
	check(APIData);
	
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	
	Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::ListFleets_Response);
	
	const FString APIUrl = APIData->GEtAPIEndPoint(GameplayServerTags::GameSessionsAPI::ListFleets);
	
	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->ProcessRequest();
	
	UE_LOG(LogTemp, Warning, TEXT("ListFleets_Response Made"));
}

void UPTWGameLiftSubsystem::JoinGameSession()
{
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::FindOrCreateGameSession_Response);
	
	const FString APIUrl = APIData->GEtAPIEndPoint(GameplayServerTags::GameSessionsAPI::FindOrCreateGameSession);
	
	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->ProcessRequest();
}

void UPTWGameLiftSubsystem::CreateGameSession()
{
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::CreateGameSession_Response);
	
	const FString APIUrl = APIData->GEtAPIEndPoint(GameplayServerTags::GameSessionsAPI::CreateGameSession);
	
	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->ProcessRequest();
}

void UPTWGameLiftSubsystem::DescribeGameSession(const FString& SessionId)
{
	if (!IsValid(APIData))
	{
		UE_LOG(LogTemp, Error, TEXT("DescribeGameSession Failed: APIData is NULL!"));
		return;
	}

	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::DescribeGameSession_Response);
	
	const FString APIUrl = APIData->GEtAPIEndPoint(GameplayServerTags::GameSessionsAPI::DescribeGameSession);
	FString RefinedURL = APIUrl;
	
	RefinedURL += TEXT("?gameSessionId=") + FGenericPlatformHttp::UrlEncode(SessionId);
	
	// TMap<FString, FString> Params = {
	// 	{ TEXT("gameSessionId"), SessionId }
	// };
	// const FString Content = SerializeJsonContent(Params);
	// Request->SetContentAsString(Content);
	
	Request->SetURL(RefinedURL);
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->ProcessRequest();
}

void UPTWGameLiftSubsystem::CreatePlayerSession(const FString& PlayerId, const FString& GameSessionId)
{
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::CreatePlayerSession_Response);
	
	const FString APIUrl = APIData->GEtAPIEndPoint(GameplayServerTags::GameSessionsAPI::CreatePlayerSession);
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

void UPTWGameLiftSubsystem::SearchGameSessions()
{
	FHttpModule* Http = &FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
	
	Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::SearchGameSessions_Response);
	
	const FString APIUrl = APIData->GEtAPIEndPoint(GameplayServerTags::GameSessionsAPI::SearchGameSessions);
	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->ProcessRequest();
	UE_LOG(LogTemp, Log, TEXT("Searching for game sessions..."));
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

void UPTWGameLiftSubsystem::CreateGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Create Game Session Response Received");
	if (!bWasSuccessful || !Response.IsValid() || Response->GetResponseCode() != 200)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateGameSession Failed."));
		return;
	}
	
	FPTWGameLiftGameSession GameSession;
	if (ParseDataFromJson<FPTWGameLiftGameSession>(Response->GetContentAsString(), GameSession))
	{
		const FString GameSessionId = GameSession.GameSessionId;
		const FString GameSessionStatus = GameSession.Status;
		TryJoinGameSession(GameSessionStatus, GameSessionId);
	}
}


void UPTWGameLiftSubsystem::DescribeGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response,
                                                         bool bWasSuccessful)
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
		TryJoinGameSession(GameSessionStatus, GameSessionId);
	}
}

void UPTWGameLiftSubsystem::TryJoinGameSession(const FString& Status, const FString& SessionId)
{
	if (Status.Equals(TEXT("ACTIVE")))
	{
		UE_LOG(LogTemp, Error, TEXT("Found active Game Session. Creating a Player Session..."));
		CreatePlayerSession(GetUniquePlayerId(), SessionId);
	}
	else if (Status.Equals(TEXT("ACTIVATING")))
	{
		// FTimerDelegate CheckSessionDelegate;
		// CheckSessionDelegate.BindLambda([this, SessionId]()
		// {
		// 	DescribeGameSession(SessionId); 
		// });
		// LocalPlayerController->GetWorldTimerManager().SetTimer(CreateSessionTimer, CheckSessionDelegate, 2.0f, false);
		if (APlayerController* LocalPlayerController = GEngine->GetFirstLocalPlayerController(GetWorld()))
		{
			LocalPlayerController->GetWorldTimerManager().SetTimer(CreateSessionTimer,
			                                                       [this, SessionId]()
			                                                       {
				                                                       DescribeGameSession(SessionId);
			                                                       }, 2.0f, false);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Unknown Status %s"), *Status);
	}
}

void UPTWGameLiftSubsystem::CreatePlayerSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("Create PlayerSession Response Received"));
	
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "CreatePlayerSession_Response");
	
	FPTWGameLiftPlayerSession PlayerSession;
	if (ParseDataFromJson<FPTWGameLiftPlayerSession>(Response->GetContentAsString(), PlayerSession))
	{
		const FString IpAndPort = PlayerSession.IpAddress + TEXT(":") + FString::FromInt(PlayerSession.Port);
		const FName Address = FName(*IpAndPort);
		UGameplayStatics::OpenLevel(this, Address);
	}
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



void UPTWGameLiftSubsystem::FindOrCreateGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Find or Create GAme Session Response Received");
	
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		if (ContainErrors(JsonObject))
		{
			;
		}
		
		FPTWGameLiftGameSession GameSession;
		FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), &GameSession);
		
		const FString GameSessionId = GameSession.GameSessionId;
		const FString GameSessionStatus = GameSession.Status;
		TryJoinGameSession(GameSessionStatus, GameSessionId);
	}
}
