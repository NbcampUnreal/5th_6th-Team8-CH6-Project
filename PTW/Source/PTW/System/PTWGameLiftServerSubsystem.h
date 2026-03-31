// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#if WITH_GAMELIFT
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/OnlineSessionInterface.h"
#endif
#include "Subsystems/GameInstanceSubsystem.h"
#include "PTWGameLiftServerSubsystem.generated.h"

class UPTWAPIData;
class FGameLiftServerSDKModule;

DECLARE_DELEGATE_OneParam(FOnUpdateSessionStateCompleted, const FString&);

/**
 * 
 */
UCLASS()
class PTW_API UPTWGameLiftServerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UPTWGameLiftServerSubsystem();
	
protected:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UPTWAPIData> ServerAPIData;
	
#if WITH_GAMELIFT // 서버 전용 로직
public:
	static UPTWGameLiftServerSubsystem* Get(const UObject* WorldContextObject);
	
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
public:
	FORCEINLINE FGameLiftServerSDKModule* GetGameLiftSdkModule() const { return GameLiftSdkModule; };
	IOnlineSessionPtr GetSessionInterface() const;
	
	void SetGameLiftSdkModule(FGameLiftServerSDKModule* InGameLiftSdkModule) { GameLiftSdkModule = InGameLiftSdkModule; };
	// void SetInGameSession(const Aws::GameLift::Server::Model::GameSession& NewGameSession) { InGameSession = NewGameSession; };
	
	void SetupMapLoadDelegateHandle();
	virtual void OnMapLoaded(UWorld* LoadedWorld);
	
	// 현재 세션을 Ready(준비됨) 상태로
    void UpdateSessionToReady();
	
	void ActivateSessionAndUpdate();
	
	void UpdatePlayerCount(FString Action);
	
	// 플레이어 세션이 유효한지 검증
	bool AcceptPlayerSession(FString PlayerSessionId);
	
	// 종료한 플레이어 세션을 제거
	void RemovePlayerSession(FString PlayerSessionId);
	void ExitGameSession();
	
	void UpdateSessionState(FString Action = TEXT("ACTIVE"));
	
protected:
	void OnUpdateSessionToReadyComplete(FName SessionName, bool bWasSuccessful);
	void ActivateSessionAndUpdate_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void UpdatePlayerCount_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void UpdateSessionState_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, const FString Action);

protected:
	FGameLiftServerSDKModule* GameLiftSdkModule;
	// Aws::GameLift::Server::Model::GameSession InGameSession;
	
public:
	FTimerHandle UpdateSessionStateTimer;
	FOnUpdateSessionStateCompleted OnUpdateSessionStateCompleted;
	
private:
	FDelegateHandle MapLoadDelegateHandle;
	FDelegateHandle UpdateSessionCompleteDelegateHandle;
#endif
};
