// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Session/PTWSessionConfig.h"
#include "PTWSessionSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FOnlineSessionSearchResultBP
{
	GENERATED_USTRUCT_BODY()
	FOnlineSessionSearchResult OnlineSessionSearchResult;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionSearchComplete, const TArray<FOnlineSessionSearchResultBP>&, SearchResult);

UCLASS()
class PTW_API UPTWSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	FORCEINLINE IOnlineSessionPtr GetSessionInterface() const { return SessionInterface; };
	FORCEINLINE TSharedPtr<FOnlineSessionSearch> GetSessionSearch() const {return SessionSearch; };
	
	// 온라인 서브시스템이 스팀인지 체크
	UFUNCTION(BlueprintCallable, Category = "Session")
	bool IsUsingSteamSubsystem();
	
	// 세셩 생성
	UFUNCTION(BlueprintCallable, Category = "Session")
	void CreateGameSession(FPTWSessionConfig SessionConfig);
	
	// 세션 참여
	UFUNCTION(BlueprintCallable, Category = "Session")
	void JoinGameSession(const FOnlineSessionSearchResultBP& SearchResult);
	
	// 세션 탐색
	UFUNCTION(BlueprintCallable, Category = "Session")
	void FindGameSession();
	
	// 데디케이티드 서버 생성 (unused)
	UFUNCTION(BlueprintCallable, Category = "Session")
	void LaunchDedicatedServer(FPTWSessionConfig SessionConfig);
	
	// 리슨서버로 레벨 이동
	UFUNCTION(BlueprintCallable, Category = "Session")
	void CreateListenLevel(FName MapName, FPTWSessionConfig SessionConfig);
	
	// 세션 이탈 & 종료
	void LeaveGameSession();
	
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	// 세션 생성 성공 시 호출
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful, FPTWSessionConfig SessionConfig);
	
	// 네트워크 오류가 발생했을 시 호출
	void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);
	
	// 세션 정리가 완료됐을 시 호출
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	
	// 세션 참여가 완료됐을 시 호출
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	
	// 세션 탐색이 완료됐을 시 호출
	void OnFindSessionsComplete(bool bWasSuccessful);

protected:
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	
public:
	FOnSessionSearchComplete OnSessionSearchComplete;
	
protected:
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FDelegateHandle DestroySessionDelegateHandle;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
};
