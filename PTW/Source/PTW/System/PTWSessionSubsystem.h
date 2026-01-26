// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FindSessionsCallbackProxy.h"
#include "PTWSessionSubsystem.generated.h"
/**
 * 
 */

UCLASS(Abstract, Blueprintable)
class PTW_API UPTWSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// 로비세션 생성 함수
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Session")
	void CreateLobbySession(const TArray<FSessionPropertyKeyPair>& LobbySettings, 
		int32 MaxPlayers, bool bisPrivate = false);
	
	// 로비세션 참여 함수
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Session")
	void JoinLobbySession(const FBlueprintSessionResult& SessionResult);
	
	// 로비세션 탐색 함수 (내부 로직은 비동기로 처리)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Session")
	void FindLobbySession();
	
	// 로비세션 탐색(비동기로 처리)이 성공했을 때 호출될 함수
	UFUNCTION(BlueprintCallable, Category = "Session")
	void OnFindLobbiesComplete(const TArray<FBlueprintSessionResult>& SessionResults);
	
	// 리슨서버로 레벨오픈하는 함수
	UFUNCTION(BlueprintCallable, Category = "Session")
	void CreateListenServer(FName MapName);
	
	// 로비세션 탐색결과를 송신할 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Session")
	FBlueprintFindSessionsResultDelegate OnFindLobbiesCompleteDelegate;
};
