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
	// 미니 게임이 끝나면 데이터 삭제 
	UPROPERTY(BlueprintReadOnly)
	TArray<FPTWChaosItemEntry> ChaosItemEntries;

	UPROPERTY(BlueprintReadOnly)
	TArray<FName> PlayedMapRowNames;

	UPROPERTY(BlueprintReadOnly)
	TArray<FPTWLastWinnerInfo> LastWinnerInfos;
};
