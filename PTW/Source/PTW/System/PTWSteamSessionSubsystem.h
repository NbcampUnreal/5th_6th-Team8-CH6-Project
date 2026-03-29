// Fill out your copyright notice in the Description page of Project Settings.

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
DECLARE_MULTICAST_DELEGATE_OneParam(FOnFindByIdGameSessionComplete, const FOnlineSessionSearchResultBP&);

UCLASS()
class PTW_API UPTWSteamSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UPTWSteamSessionSubsystem* Get(const UObject* WorldContextObject);

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	FORCEINLINE IOnlineSessionPtr GetSessionInterface() const { return SessionInterface; };
	
	// 온라인 서브시스템이 스팀인지 체크
	UFUNCTION(BlueprintCallable, Category = "Session")
	bool IsUsingSteamSubsystem();
	
	// 현재 세션의 SteamID (CSteamID) 반환
	FString GetSteamServerID();
	
	// 현재 세션에 접속가능한 최대 플레이어 수 반환
	int32 GetMaxPlayers();
	
	// 현재 세션에 설정된 최대 라운드 수 반환
	int32 GetMaxRounds();
	
	// 세셩 생성
	UFUNCTION(BlueprintCallable, Category = "Session")
	void CreateGameSession(FPTWSessionConfig SessionConfig, bool bTravelOnSuccess);
	
	// 세션 참여
	UFUNCTION(BlueprintCallable, Category = "Session")
	void JoinGameSession(const FOnlineSessionSearchResultBP& SearchResult, const FString Options = TEXT(""));
	
	// 세션 탐색 선작업
	UFUNCTION(BlueprintCallable, Category = "Session")
	void FindGameSession();
	
	// 세션 검색
	UFUNCTION(BlueprintCallable, Category = "Session")
	void SearchForGameSessions();
	
	// Id기반 세션 검색
	UFUNCTION(BlueprintCallable, Category = "Session")
	void FindByIdGameSession(const FString& SteamId);

	// 리슨서버로 레벨 이동
	UFUNCTION(BlueprintCallable, Category = "Session")
	void OpenServerLevel(FName MapName, FPTWSessionConfig SessionConfig);
	
	// 세션 이탈 & 종료
	void LeaveGameSession();
	
	// 스팀 세션 이탈 함수
	bool UnregisterPlayer(FName SessionName, const FUniqueNetId& PlayerId);
	
	// 호스트 전용 세션 종료 함수
	void ExitGameSession();
	
	UFUNCTION(BlueprintCallable, Category = "Session")
	void QuickMatchGameSession();
	
	void UpdateGameSeesionPlayerCount(int32 CurrentPlayerCount);

protected:
	// 세션 생성 성공 시 호출
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful, FPTWSessionConfig SessionConfig, bool bTravelOnSuccess);
	
	// 네트워크 오류가 발생했을 시 호출
	void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);
	
	// 세션 정리가 완료됐을 시 호출
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	
	// 세션 참여가 완료됐을 시 호출
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result, const FString Options);
	
	// 세션 탐색이 완료됐을 시 호출
	void OnFindSessionsComplete(bool bWasSuccessful);
	
	// id 기반 세션 탐색
	void OnFindByIdOnComplete(bool bWasSuccessful, const FString SteamId);
	
	UFUNCTION(BlueprintCallable, Category = "Session")
	void OnQuickMatchFindSessionsComplete();
	
public:
protected:
	IOnlineSessionPtr SessionInterface;
	TArray<FOnlineSessionSearchResultBP> BPSearchResults;
	TQueue<TSharedPtr<FOnlineSessionSearch>> SessionSearchQueue;
	
private:
	
public:
	FOnSessionSearchComplete OnSessionSearchComplete;
	FOnAllSessionSearchFinished OnAllSessionSearchFinished;
	FOnSteamSessionMessageReceived OnSteamSessionMessageReceived;
	FOnFindByIdGameSessionComplete OnFindByIdGameSessionComplete;
protected:
	FDelegateHandle SteamLoginCompletedHandle;
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FDelegateHandle DestroySessionDelegateHandle;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	
private:
};
