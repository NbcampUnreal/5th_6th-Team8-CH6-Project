// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWGameData.generated.h"

/**
 * 
 */
USTRUCT(Blueprintable)
struct FPTWChaosItemEntry 
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName ItemId;

	UPROPERTY(BlueprintReadOnly)
	FString PlayerName;
};

USTRUCT(Blueprintable)
struct FPTWLastWinnerInfo 
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString WinnerId;
	
};

USTRUCT(Blueprintable)
struct FPTWGameData 
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentRound;
	
	// 미니 게임이 끝나면 데이터 삭제
	UPROPERTY(BlueprintReadOnly)
	TArray<FPTWChaosItemEntry> ChaosItemEntries;

	UPROPERTY(BlueprintReadOnly)
	TArray<FName> PlayedMapRowNames;

	UPROPERTY(BlueprintReadOnly)
	TArray<FPTWLastWinnerInfo> LastWinnerInfos;
};

USTRUCT(Blueprintable)
struct FPTWBaseRankingData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 Rank =0;
	
	UPROPERTY(BlueprintReadOnly)
	FString PlayerName = "";
	
	UPROPERTY(BlueprintReadOnly)
	int32 Score = 0;
	
	UPROPERTY(BlueprintReadWrite)
	TArray<FString> InventoryItemIDs;

	UPROPERTY(BlueprintReadOnly)
	bool bLeftGame;
};

USTRUCT(Blueprintable)
struct FPTWLobbyRankingData : public FPTWBaseRankingData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	int32 Gold;
	
};

USTRUCT(Blueprintable)
struct FPTWMiniGameRankingData : public FPTWBaseRankingData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	int32 Kill = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 Death = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 DeathOrder = -1;
};
