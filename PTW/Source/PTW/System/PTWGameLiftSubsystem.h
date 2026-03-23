#pragma once
#include "CoreMinimal.h"
#include "type_traits"
#include "JsonObjectConverter.h"
#include "Session/PTWSessionConfig.h"
#include "Interfaces/IHttpRequest.h"
#include "Subsystems/GameInstanceSubsystem.h"
#if WITH_GAMELIFT
#include "GameLiftServerSDK.h"
#endif
#include "PTWGameLiftSubsystem.generated.h"

class UPTWAPIData;
class FJsonObject;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameLiftSessionSearchComplete, const TArray<FPTWGameLiftGameSession>&, SearchResults);

UCLASS()
class PTW_API UPTWGameLiftSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UPTWGameLiftSubsystem();
	
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
public:
	FString SerializeJsonContent(const TMap<FString, FString>& Params);
	template <typename T>
	bool ParseDataFromJson(const FString& JsonString, T& OutStruct)
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
	bool ParseDataArrayFromJson(const FString& JsonString, TArray<T>& OutArray)
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
	void RequestListFleets();
	void CreateGameSession(FPTWSessionConfig& SessionConfig);
	void CheckSessionStatus(const FString& SessionId);
	void DescribeGameSession(const FString& SessionId);
	void CreatePlayerSession(const FString& PlayerId, const FString& GameSessionId);
	void SearchGameSessions();
	
protected:
	bool ContainErrors(TSharedPtr<FJsonObject> JsonObject);
	void DumpMetadata(TSharedPtr<FJsonObject> JsonObject);
	
	void CreateGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void CheckSessionStatus_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void DescribeGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void TryJoinGameSession(const FString& SessionId, const FString& SteamId, const FString& Status);
	void CreatePlayerSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void SearchGameSessions_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	
private:
	FString GetUniquePlayerId() const;
	void ListFleets_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

protected:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UPTWAPIData> ClientAPIData;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UPTWAPIData> ServerAPIData;
public:
	FOnGameLiftSessionSearchComplete OnSessionSearchComplete;
	
private:
	FTimerHandle CreateSessionTimer;
	FDelegateHandle MapLoadDelegateHandle;

#if WITH_GAMELIFT // 서버 전용 로직
public:
	void ReportServerInfoToBackend();
	virtual void OnMapLoaded(UWorld* LoadedWorld);
	FORCEINLINE FGameLiftServerSDKModule* GetGameLiftSdkModule() const { return GameLiftSdkModule; };
	void SetupMapLoadDelegateHandle();
	void SetGameLiftSdkModule(FGameLiftServerSDKModule* InGameLiftSdkModule) { GameLiftSdkModule = InGameLiftSdkModule; };
	void SetInGameSession(const Aws::GameLift::Server::Model::GameSession& NewGameSession) { InGameSession = NewGameSession; };
	
protected:
	void ReportServerInfoToBackend_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	
protected:
	FGameLiftServerSDKModule* GameLiftSdkModule;
	Aws::GameLift::Server::Model::GameSession InGameSession;
	
#endif
};
