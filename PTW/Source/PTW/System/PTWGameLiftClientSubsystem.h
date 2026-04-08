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
struct FOnlineSessionSearchResultBP;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameLiftSessionSearchComplete, const TArray<FPTWGameSessionListsTable>&, SearchResults);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameLiftSessionMessageReceived, const FText&, Message);

UCLASS()
class PTW_API UPTWGameLiftClientSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UPTWGameLiftClientSubsystem();
	static UPTWGameLiftClientSubsystem* Get(const UObject* WorldContextObject);
	
	FString GetUniquePlayerId() const;
	void CreateGameSession(FPTWSessionConfig& SessionConfig);
	void CheckSessionStatus(const FString& SessionId, bool bIsLoop = false);
	void CreatePlayerSession(const FString& PlayerId, const FString& GameSessionId);
	void SearchGameSessions();
	void SearchQuickSession();
	void FindByIdAndJoinSession(const FString& SteamId, const FString& Options);
	
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void CreateGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void CheckSessionStatus_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void CheckSessionStatusLoop_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FString SessionId);
	void WaitForSessionActivation(const FString& SessionId);
	void CreatePlayerSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void SearchGameSessions_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void SearchQuickSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
protected:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UPTWAPIData> ClientAPIData;

public:
	FOnGameLiftSessionSearchComplete OnSessionSearchComplete;
	FOnGameLiftSessionMessageReceived OnGameLiftSessionMessageReceived;
private:
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FTimerHandle CheckSessionLitmitTimer;
};
