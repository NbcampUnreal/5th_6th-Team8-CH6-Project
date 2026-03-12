// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWHTTPRequestManager.h"

#include "GameplayServerTags.h"
#include "PTWAPIData.h"
#include "HttpModule.h"
#include "JsonObjectConverter.h"
#include "PTWHTTPRequestTypes.h"
#include "GameFramework/PlayerState.h"
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
		
		FGameLiftGameSession GameSession;
		FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), &GameSession);
		
		const FString GameSessionId = GameSession.GameSessionId;
		const FString GameSessionStatus = GameSession.Status;
		HandleGameSessionStatus(GameSessionStatus, GameSessionId);
	}
}

void UPTWHTTPRequestManager::CreateGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response,
	bool bWasSuccessful)
{
	TSharedPtr<FJsonObject> JsonObject;
	FGameLiftGameSession GameSession;
	FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), &GameSession);
	
	const FString GameSessionId = GameSession.GameSessionId;
	const FString GameSessionStatus = GameSession.Status;
	HandleGameSessionStatus(GameSessionStatus, GameSessionId);
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
	Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::CreatePlayerSession_Response);
	
	const FString APIUrl = APIData->GEtAPIEndPoint(GameplayServerTags::GameSessionsAPI::CreateGameSession);
	
	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("POST"));
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
		CreateSessionDelegate.BindUObject(this, &ThisClass::CreateGameSession);
		if (APlayerController* LocalPlayerController = GEngine->GetFirstLocalPlayerController(GetWorld()))
		{
			LocalPlayerController->GetWorldTimerManager().SetTimer(CreateSessionTimer, CreateSessionDelegate, 0.5f, false);
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
	
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		if (ContainErrors(JsonObject))
		{
			;
		}
		
		FGameLiftPlayerSession PlayerSession;
		FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), &PlayerSession);
		
		const FString IpAndPort = PlayerSession.IPAddress + TEXT(":") + FString::FromInt(PlayerSession.Port);
		const FName Address = FName(*IpAndPort);
		UGameplayStatics::OpenLevel(this, Address);
	}
}
