// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "UObject/Object.h"
#include "PTWHTTPRequestManager.generated.h"

class UPTWAPIData;
class FJsonObject;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnListFleetsResponseReceived, const FPTWListFleetsResponse&, ListFleetsResponse, bool, bwasSuccessful);

UCLASS(Blueprintable, BlueprintType)
class PTW_API UPTWHTTPRequestManager : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UPTWAPIData> APIData;
	
	void RequestListFleets();
	
	void ListFleets_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void FindOrCreateGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void CreateGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void JoinGameSession();
	UPROPERTY()
	FOnListFleetsResponseReceived OnListFleetsResponseReceived;
protected:
	
	bool ContainErrors(TSharedPtr<FJsonObject> JsonObject);
	void DumpMetadata(TSharedPtr<FJsonObject> JsonObject);
	
private:
	FString GetUniquePlayerId() const;
	
	void HandleGameSessionStatus(const FString& Status, const FString& SessionId);
	void TryCreatePlayerSession(const FString& PlayerId, const FString& GameSessionId);
	
	FTimerHandle CreateSessionTimer;
};
