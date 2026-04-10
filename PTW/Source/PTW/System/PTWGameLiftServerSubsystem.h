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
 * 서버 전용 로직을 관리하는 게임리프트 서브 시스템입니다.
 */
UCLASS()
class PTW_API UPTWGameLiftServerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UPTWGameLiftServerSubsystem();
	static UPTWGameLiftServerSubsystem* Get(const UObject* WorldContextObject);
	
#if WITH_GAMELIFT // 서버 전용 로직
	FORCEINLINE FGameLiftServerSDKModule* GetGameLiftSdkModule() const { return GameLiftSdkModule; };
	IOnlineSessionPtr GetSessionInterface() const;
	void SetGameLiftSdkModule(FGameLiftServerSDKModule* InGameLiftSdkModule) { GameLiftSdkModule = InGameLiftSdkModule; };
	
	void SetupMapLoadDelegateHandle();
	
	void UpdateSessionToReady();
	void ActivateSessionAndUpdate();
	void UpdatePlayerCount(FString Action);
	void UpdateSessionState(FString Action = TEXT("ACTIVE"));
	bool AcceptPlayerSession(FString PlayerSessionId);
	void RemovePlayerSession(FString PlayerSessionId);
	void ExitGameSession();

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	void OnMapLoaded(UWorld* LoadedWorld);
	void OnUpdateSessionToReadyComplete(FName SessionName, bool bWasSuccessful);
	void ActivateSessionAndUpdate_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void UpdatePlayerCount_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void UpdateSessionState_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, const FString Action);

#endif
protected:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UPTWAPIData> ServerAPIData;
	
#if WITH_GAMELIFT
	FGameLiftServerSDKModule* GameLiftSdkModule;
	
public:
	FTimerHandle UpdateSessionStateTimer;
	FOnUpdateSessionStateCompleted OnUpdateSessionStateCompleted;
	
private:
	FDelegateHandle MapLoadDelegateHandle;
	FDelegateHandle UpdateSessionCompleteDelegateHandle;
#endif
};
