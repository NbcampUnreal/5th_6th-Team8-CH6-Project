#pragma once
#include "CoreMinimal.h"
#include "type_traits"
#include "JsonObjectConverter.h"
#include "Session/PTWSessionConfig.h"
#include "Interfaces/IHttpRequest.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PTWGameLiftClientSubsystem.generated.h"

class UPTWAPIData;
class FJsonObject;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameLiftSessionSearchComplete, const TArray<FPTWGameLiftGameSession>&, SearchResults);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameLiftSessionMessageReceived, const FText&, Message);

UCLASS()
class PTW_API UPTWGameLiftClientSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UPTWGameLiftClientSubsystem();
	static UPTWGameLiftClientSubsystem* Get(const UObject* WorldContextObject);
	
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
public:
	static FString SerializeJsonContent(const TMap<FString, FString>& Params);
	template <typename T>
	static bool ParseDataFromJson(const FString& JsonString, T& OutStruct)
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);
		if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
		{
			const TSharedPtr<FJsonObject>* DataObjPtr = nullptr;
			if (JsonObject->TryGetObjectField(TEXT("data"), DataObjPtr) && DataObjPtr->IsValid())
			{
				FPTWGameLiftGameSession GameSession;
				if (FJsonObjectConverter::JsonObjectToUStruct(DataObjPtr->ToSharedRef(), &GameSession))
				{
					return FJsonObjectConverter::JsonObjectToUStruct(DataObjPtr->ToSharedRef(), &OutStruct);
				}
			}
		}
		return false;
	}
	template <typename T>
	static bool ParseDataArrayFromJson(const FString& JsonString, TArray<T>& OutArray)
	{
	    TSharedPtr<FJsonObject> JsonObject;
	    TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);

	    if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
	    {
	        const TArray<TSharedPtr<FJsonValue>>* DataArrayPtr = nullptr;
	        if (JsonObject->TryGetArrayField(TEXT("data"), DataArrayPtr))
	        {
	            return FJsonObjectConverter::JsonArrayToUStruct(*DataArrayPtr, &OutArray);
	        }
	    }
	    return false;
	}
	void CreateGameSession(FPTWSessionConfig& SessionConfig);
	void CheckSessionStatus(const FString& SessionId);
	void DescribeGameSession(const FString& SessionId);
	void CreatePlayerSession(const FString& PlayerId, const FString& GameSessionId);
	void SearchGameSessions();
	FString GetUniquePlayerId() const;

protected:
	void CreateGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void CheckSessionStatus_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void DescribeGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void TryJoinGameSession(const FString& SessionId, const FString& SteamId, const FString& Status);
	void WaitForSessionActivation(const FString& SessionId);
	void CreatePlayerSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void SearchGameSessions_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

protected:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UPTWAPIData> ClientAPIData;

public:
	FOnGameLiftSessionSearchComplete OnSessionSearchComplete;
	FOnGameLiftSessionMessageReceived OnGameLiftSessionMessageReceived;
private:
	FTimerHandle CheckSessionLitmitTimer;
};
