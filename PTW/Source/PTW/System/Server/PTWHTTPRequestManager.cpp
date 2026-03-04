// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWHTTPRequestManager.h"

#include "GameplayServerTags.h"
#include "PTWAPIData.h"
#include "HttpModule.h"
#include "JsonObjectConverter.h"
#include "PTWHTTPRequestTypes.h"
#include "Interfaces/IHttpResponse.h"

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
		if(JsonObject->HasField(TEXT("$metadata")))
		{
			TSharedPtr<FJsonObject> MetaDataJsonObject = JsonObject->GetObjectField(TEXT("$metadata"));
			FPTWMetaData PTWMetaData;
			FJsonObjectConverter::JsonObjectToUStruct(MetaDataJsonObject.ToSharedRef(), &PTWMetaData);
			
			PTWMetaData.Dump();
		}
		
		FPTWListFleetsResponse ListFleetsResponse;
		FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), &ListFleetsResponse);
		ListFleetsResponse.Dump();
	}
}
