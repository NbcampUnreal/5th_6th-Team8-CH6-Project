// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FindSessionsCallbackProxy.h"
#include "PTWSessionSubsystem.generated.h"
/**
 * 
 */

struct FSessionPropertyKeyPair;

UCLASS(Abstract, Blueprintable)
class PTW_API UPTWSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Session")
	void CreateLobbySession(const TArray<FSessionPropertyKeyPair>& LobbySettings, 
		int32 MaxPlayers, bool bIsPrivate = false);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Session")
	void JoinLobbySession(const FBlueprintSessionResult& SessionResult);
	
	// 로비세션 탐색 함수 (내부 로직은 비동기로 처리)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Session")
	void FindLobbySession();
	
	// 로비세션 탐색(비동기로 처리)이 성공했을 때 호출될 함수
	UFUNCTION(BlueprintCallable, Category = "Session")
	void OnFindLobbiesComplete(const TArray<FBlueprintSessionResult>& SessionResults);
	
	UFUNCTION(BlueprintCallable, Category = "Session")
	void LaunchDedicatedServer(const TArray<FSessionPropertyKeyPair>& LobbySettings,
	int32 MaxPlayers, bool bIsPrivate);
	
	// 리슨서버로 레벨 생성하는 함수
	UFUNCTION(BlueprintCallable, Category = "Session")
	void CreateListenLevel(FName MapName);

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, 
		ENetworkFailure::Type FailureType, const FString& ErrorString);
	
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	
	FDelegateHandle DestroySessionDelegateHandle;
	
public:
	// 로비세션 탐색결과를 송신할 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Session")
	FBlueprintFindSessionsResultDelegate OnFindLobbiesCompleteDelegate;
};
