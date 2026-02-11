// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "SessionConfig.generated.h"

UENUM(BlueprintType)
enum class EPTWRoundLimit : uint8
{
	Short	UMETA(DisplayName = "ShortRound"),
	Long	UMETA(DisplayName = "LongRound")
};

static int32 GetMaxRoundsByLimit(EPTWRoundLimit Limit)
{
	switch (Limit)
	{
	case EPTWRoundLimit::Short:
		return 5;
	case EPTWRoundLimit::Long:
		return 10;
	default:
		return 0;
	}
}

namespace SessionKey
{
	inline const FName ServerName = FName(TEXT("SERVER_NAME"));
	inline const FName MapName =	FName(TEXT("MAP_NAME"));
	inline const FName MaxPlayers =	FName(TEXT("MAX_PLAYERS"));
	inline const FName MaxRounds =	FName(TEXT("MAX_ROUNDS"));
}

USTRUCT(BlueprintType)
struct FSessionConfig
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	FString ServerName = TEXT("Server");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	bool bIsDedicatedServer = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	FString MapName = TEXT("Map");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	int32 MaxPlayers = 16;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	int32 MaxRounds = 5;
};
