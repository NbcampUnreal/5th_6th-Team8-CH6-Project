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
	virtual void  OnMapLoaded(UWorld* LoadedWorld);
	void SetupMapLoadDelegateHandle();
	FString SerializeJsonContent(const TMap<FString, FString>& Params);
	template <typename T> requires (std::is_same_v<T, FPTWGameLiftGameSession> || std::is_same_v<T, FPTWGameLiftPlayerSession>)
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
	template <typename T> requires (std::is_same_v<T, FPTWGameLiftGameSession> || std::is_same_v<T, FPTWGameLiftPlayerSession>)
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
	void JoinGameSession();
	void CreateGameSession();
	void DescribeGameSession(const FString& SessionId);
	void CreatePlayerSession(const FString& PlayerId, const FString& GameSessionId);
	void SearchGameSessions();
	
protected:
	bool ContainErrors(TSharedPtr<FJsonObject> JsonObject);
	void DumpMetadata(TSharedPtr<FJsonObject> JsonObject);
	
	void CreateGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void DescribeGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void TryJoinGameSession(const FString& Status, const FString& SessionId);
	void CreatePlayerSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void SearchGameSessions_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	
private:
	FString GetUniquePlayerId() const;
	void ListFleets_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void FindOrCreateGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

protected:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UPTWAPIData> APIData;

public:
	FOnGameLiftSessionSearchComplete OnSessionSearchComplete;
	
private:
	FTimerHandle CreateSessionTimer;
	FDelegateHandle MapLoadDelegateHandle;
#if WITH_GAMELIFT
public:
	FGameLiftServerSDKModule* GetGameLiftSdkModule() const { return GameLiftSdkModule; };
	void SetGameLiftSdkModule(FGameLiftServerSDKModule* InGameLiftSdkModule) { GameLiftSdkModule = InGameLiftSdkModule; };
protected:
	FGameLiftServerSDKModule* GameLiftSdkModule;
#endif
};
