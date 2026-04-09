#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Session/PTWSessionConfig.h"
#include "PTWSteamSessionSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FOnlineSessionSearchResultBP
{
	GENERATED_USTRUCT_BODY()
	FOnlineSessionSearchResult OnlineSessionSearchResult;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionSearchComplete, const TArray<FOnlineSessionSearchResultBP>&, SearchResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllSessionSearchFinished);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSteamSessionMessageReceived, const FText&, Message);

UCLASS()
class PTW_API UPTWSteamSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UPTWSteamSessionSubsystem* Get(const UObject* WorldContextObject);
	FORCEINLINE IOnlineSessionPtr GetSessionInterface() const { return SessionInterface; };

	// 현재 Server의 SteamId를 반환
	UFUNCTION(BlueprintCallable, Category = "Session")
	FString GetSteamServerID() const;
	
	// 현재 세션에 접속가능한 최대 플레이어 수 반환
	UFUNCTION(BlueprintCallable, Category = "Session")
	int32 GetMaxPlayers();
	
	// 현재 세션에 설정된 최대 라운드 수 반환
	UFUNCTION(BlueprintCallable, Category = "Session")
	int32 GetMaxRounds();
	
	void CreateGameSession(FPTWSessionConfig SessionConfig, bool bTravelOnSuccess);
	void FindGameSession();
	void SearchForGameSessions();
	void JoinGameSession(const FOnlineSessionSearchResultBP& SearchResult, const FString Options = TEXT(""));
	void LeaveGameSession();
	void ExitGameSession();

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);
	void OpenServerLevel(FName MapName, FPTWSessionConfig SessionConfig) const;
	
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful, FPTWSessionConfig SessionConfig, bool bTravelOnSuccess);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result, const FString Options);
	void OnFindSessionsComplete(bool bWasSuccessful);

protected:
	IOnlineSessionPtr SessionInterface;
	TArray<FOnlineSessionSearchResultBP> BPSearchResults;
	TQueue<TSharedPtr<FOnlineSessionSearch>> SessionSearchQueue;

public:
	FOnSessionSearchComplete OnSessionSearchComplete;
	FOnAllSessionSearchFinished OnAllSessionSearchFinished;
	FOnSteamSessionMessageReceived OnSteamSessionMessageReceived;
	
protected:
	FDelegateHandle SteamLoginCompletedHandle;
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FDelegateHandle DestroySessionDelegateHandle;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
};
