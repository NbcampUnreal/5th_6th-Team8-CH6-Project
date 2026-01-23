// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FindSessionsCallbackProxy.h"
#include "PTWSessionSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FLobbySettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LobbySettings")
	FString LobbyName = "";
	
};

/**
 * 
 */

UCLASS(Abstract, Blueprintable)
class PTW_API UPTWSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Session")
	void CreateLobbySession(const FLobbySettings LobbySettings);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Session")
	void JoinLobbySession(const FBlueprintSessionResult SessionResult);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Session")
	void FindLobbySession();
	
	UFUNCTION(BlueprintCallable, Category = "Session")
	void OnFindLobbiesComplete(const TArray<FBlueprintSessionResult>& SessionResults);
	
	UFUNCTION(BlueprintCallable, Category = "Session")
	void CreateListenServer(FName MapName);
	
	UFUNCTION(BlueprintCallable, Category = "Session")
	FORCEINLINE TArray<FBlueprintSessionResult> GetLobbyList() const { return LobbyList; };

	UPROPERTY(BlueprintAssignable, Category = "Session")
	FBlueprintFindSessionsResultDelegate OnFindLobbiesCompleteDelegate;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	TArray<FBlueprintSessionResult> LobbyList;
};
