// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "FindSessionsCallbackProxy.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "PTWSessionSubsystem.generated.h"
/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionSearchComplete, const TArray<FBlueprintSessionResult>&, SearchResult);

UCLASS()
class PTW_API UPTWSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Session")
	void CreateGameSession(FSessionConfig SessionConfig);
	
	UFUNCTION(BlueprintCallable, Category = "Session")
	void JoinGameSession(const FBlueprintSessionResult& SearchResult);
	
	UFUNCTION(BlueprintCallable, Category = "Session")
	void FindGameSession();
	
	UFUNCTION(BlueprintCallable, Category = "Session")
	void LaunchDedicatedServer(const TArray<FSessionPropertyKeyPair>& LobbySettings,
	int32 MaxPlayers, bool bIsPrivate);
	
	// 리슨서버로 레벨 생성하는 함수
	UFUNCTION(BlueprintCallable, Category = "Session")
	void CreateListenLevel(FName MapName, FSessionConfig SessionConfig);
	
	void LeaveGameSession();
	
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful, FSessionConfig SessionConfig);
	void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnFindSessionsComplete(bool bWasSuccessful);

public:
	FOnSessionSearchComplete OnSessionSearchComplete;
	
protected:
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FDelegateHandle DestroySessionDelegateHandle;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	
};
