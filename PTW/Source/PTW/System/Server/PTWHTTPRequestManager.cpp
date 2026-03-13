// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWHTTPRequestManager.h"
#include "GameplayServerTags.h"
#include "PTWAPIData.h"
#include "HttpModule.h"
#include "JsonObjectConverter.h"
#include "PTWHTTPRequestTypes.h"
#include "GameFramework/PlayerState.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Interfaces/IHttpResponse.h"
#include "Kismet/GameplayStatics.h"
#include "System/Session/PTWSessionConfig.h"

void UPTWHTTPRequestManager::RequestListFleets()
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

void UPTWHTTPRequestManager::ListFleets_Response(FHttpRequestPtr Request, FHttpResponsePtr Response,
	bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("ListFleets_Response Received"));
	
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		if (ContainErrors(JsonObject))
		{
			OnListFleetsResponseReceived.Broadcast(FPTWListFleetsResponse(), false);
			return;
		}
		
		DumpMetadata(JsonObject);
		
		FPTWListFleetsResponse ListFleetsResponse;
		FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), &ListFleetsResponse);
		ListFleetsResponse.Dump();
		
		OnListFleetsResponseReceived.Broadcast(ListFleetsResponse, true);
	}
}

void UPTWHTTPRequestManager::FindOrCreateGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response,
	bool bWasSuccessful)
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
		HandleGameSessionStatus(GameSessionStatus, GameSessionId);
	}
}

void UPTWHTTPRequestManager::CreateGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response,
	bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Create Game Session Response Received");
	if (!bWasSuccessful || !Response.IsValid() || Response->GetResponseCode() != 200)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateGameSession Failed."));
		return;
	}
	
	TSharedPtr<FJsonObject> OuterJsonObject;
	TSharedRef<TJsonReader<>> OutJsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	// FString RawResponse = Response->GetContentAsString();
	// UE_LOG(LogTemp, Warning, TEXT("Raw Server Response: %s"), *RawResponse);
	if (!FJsonSerializer::Deserialize(OutJsonReader, OuterJsonObject)) return;
	FString BodyString;
	if (!OuterJsonObject->TryGetStringField(TEXT("body"), BodyString))
	{
		UE_LOG(LogTemp, Error, TEXT("응답에 'body' 필드가 없습니다."));
		return;
	}
	
	TSharedPtr<FJsonObject> InnerJsonObject;
	TSharedRef<TJsonReader<>> InnerReader = TJsonReaderFactory<>::Create(BodyString);
	if (!FJsonSerializer::Deserialize(InnerReader, InnerJsonObject)) return;
	
	FPTWGameLiftGameSession GameSession;
	if (FJsonObjectConverter::JsonObjectToUStruct(InnerJsonObject.ToSharedRef(), &GameSession))
	{
		const FString GameSessionId = GameSession.GameSessionId;
		const FString GameSessionStatus = GameSession.Status;
		HandleGameSessionStatus(GameSessionStatus, GameSessionId);
	}
}

void UPTWHTTPRequestManager::JoinGameSession()
{
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::FindOrCreateGameSession_Response);
	
	const FString APIUrl = APIData->GEtAPIEndPoint(GameplayServerTags::GameSessionsAPI::FindOrCreateGameSession);
	
	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->ProcessRequest();
}

void UPTWHTTPRequestManager::CreateGameSession()
{
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::CreateGameSession_Response);
	
	const FString APIUrl = APIData->GEtAPIEndPoint(GameplayServerTags::GameSessionsAPI::CreateGameSession);
	
	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->ProcessRequest();
}

void UPTWHTTPRequestManager::DescribeGameSession(const FString& SessionId)
{
	if (!APIData)
	{
		UE_LOG(LogTemp, Error, TEXT("DescribeGameSession Failed: APIData is NULL!"));
		return;
	}

	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::DescribeGameSession_Response);
	
	const FString BaseAPIUrl = APIData->GEtAPIEndPoint(GameplayServerTags::GameSessionsAPI::DescribeGameSession);
	const FString EncodedSessionId = FGenericPlatformHttp::UrlEncode(SessionId);
	const FString RefinedURL = BaseAPIUrl + TEXT("?gameSessionId=") + EncodedSessionId;
	// const FString RefinedURL = BaseAPIUrl + TEXT("?gameSessionId=") + SessionId;
	UE_LOG(LogTemp, Warning, TEXT("요청할 URL: %s"), *RefinedURL);
	
	Request->SetURL(RefinedURL);
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->ProcessRequest();
}

bool UPTWHTTPRequestManager::ContainErrors(TSharedPtr<FJsonObject> JsonObject)
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

void UPTWHTTPRequestManager::DumpMetadata(TSharedPtr<FJsonObject> JsonObject)
{
	if(JsonObject->HasField(TEXT("$metadata")))
	{
		TSharedPtr<FJsonObject> MetaDataJsonObject = JsonObject->GetObjectField(TEXT("$metadata"));
		FPTWMetaData PTWMetaData;
		FJsonObjectConverter::JsonObjectToUStruct(MetaDataJsonObject.ToSharedRef(), &PTWMetaData);
			
		PTWMetaData.Dump();
	}
}

FString UPTWHTTPRequestManager::SerializeJsonContent(const TMap<FString, FString>& Params)
{
	TSharedPtr<FJsonObject> ContentJsonObject = MakeShareable(new FJsonObject());
	
	// Lambda 함수와 동일한 필드로 설정.
	// ContentJsonObject->SetStringField(TEXT("playerId"), PlayerId);
	// ContentJsonObject->SetStringField(TEXT("gameSessionId"), GameSessionId);
	for (const auto& Param : Params)
	{
		ContentJsonObject->SetStringField(Param.Key, Param.Value);
	}
	
	FString Content;
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&Content);
	FJsonSerializer::Serialize(ContentJsonObject.ToSharedRef(), JsonWriter);
	
	return Content;
}

FString UPTWHTTPRequestManager::GetUniquePlayerId() const
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

void UPTWHTTPRequestManager::HandleGameSessionStatus(const FString& Status, const FString& SessionId)
{
	if (Status.Equals(TEXT("ACTIVE")))
	{
		UE_LOG(LogTemp, Error, TEXT("Found active Game Session. Creating a Player Session..."));
		TryCreatePlayerSession(GetUniquePlayerId(), SessionId);
	}
	else if (Status.Equals(TEXT("ACTIVATING")))
	{
		// UE_LOG(LogTemp, Error, TEXT("Game Session Status: ACTIVATING"));
		
		FTimerDelegate CreateSessionDelegate;
		// CreateSessionDelegate.BindLambda([this]()
		// {
		// 	JoinGameSession();
		// });
		// CreateSessionDelegate.BindUObject(this, &ThisClass::CreateGameSession);
		// if (APlayerController* LocalPlayerController = GEngine->GetFirstLocalPlayerController(GetWorld()))
		// {
		// 	LocalPlayerController->GetWorldTimerManager().SetTimer(CreateSessionTimer, CreateSessionDelegate, 2.0f, false);
		// }
		
		FTimerDelegate CheckSessionDelegate;
		CheckSessionDelegate.BindLambda([this, SessionId]()
		{
			DescribeGameSession(SessionId); 
		});
		if (APlayerController* LocalPlayerController = GEngine->GetFirstLocalPlayerController(GetWorld()))
		{
			LocalPlayerController->GetWorldTimerManager().SetTimer(CreateSessionTimer, CheckSessionDelegate, 2.0f, false);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Unknown Status %s"), *Status);
	}
}

void UPTWHTTPRequestManager::TryCreatePlayerSession(const FString& PlayerId, const FString& GameSessionId)
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

void UPTWHTTPRequestManager::CreatePlayerSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response,
	bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("Create PlayerSession Response Received"));
	
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "CreatePlayerSession_Response");
	FString RawResponse = Response->GetContentAsString();
	
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(RawResponse);
	
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		FPTWGameLiftPlayerSession PlayerSession;
		FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), &PlayerSession);
		
		const FString IpAndPort = PlayerSession.IpAddress + TEXT(":") + FString::FromInt(PlayerSession.Port);
		const FName Address = FName(*IpAndPort);
		UGameplayStatics::OpenLevel(this, Address);
	}
}

void UPTWHTTPRequestManager::DescribeGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response,
	bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid() || Response->GetResponseCode() != 200)
	{
		UE_LOG(LogTemp, Error, TEXT("DescribeGameSession 통신 실패."));
		return;
	}
    
	FString RawResponse = Response->GetContentAsString();
	
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(RawResponse);
	
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		FPTWGameLiftGameSession GameSession;
		if (FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), &GameSession))
		{
			const FString GameSessionId = GameSession.GameSessionId;
			const FString GameSessionStatus = GameSession.Status;
	      
			UE_LOG(LogTemp, Log, TEXT("세션 상태 확인됨: %s"), *GameSessionStatus);

			HandleGameSessionStatus(GameSessionStatus, GameSessionId);
		}
	}
}

void UPTWHTTPRequestManager::SearchGameSessions()
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

void UPTWHTTPRequestManager::SearchGameSessions_Response(FHttpRequestPtr Request, FHttpResponsePtr Response,
	bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("HTTP Request failed!"));
		return;
	}

	if (EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			FString Status = JsonObject->GetStringField(TEXT("status"));
            
			if (Status == TEXT("success"))
			{
				TArray<TSharedPtr<FJsonValue>> Sessions = JsonObject->GetArrayField(TEXT("gameSessions"));
                
				for (auto& SessionValue : Sessions)
				{
					TSharedPtr<FJsonObject> SessionObj = SessionValue->AsObject();
					FString SessionId = SessionObj->GetStringField(TEXT("GameSessionId"));
					int32 CurrentPlayers = SessionObj->GetIntegerField(TEXT("CurrentPlayerSessionCount"));
                    
					UE_LOG(LogTemp, Log, TEXT("Found Session: %s (Players: %d)"), *SessionId, CurrentPlayers);
				}

				if (Sessions.Num() == 0)
				{
					UE_LOG(LogTemp, Warning, TEXT("No active sessions found."));
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Server returned error: %d"), Response->GetResponseCode());
	}
}
