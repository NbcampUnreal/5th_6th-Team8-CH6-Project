// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "PTWSessionConfig.generated.h"

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

namespace PTWSessionKey
{
	inline const FName ServerName = FName(TEXT("SERVER_NAME"));
	inline const FName MapName =	FName(TEXT("MAP_NAME"));
	inline const FName MaxPlayers =	FName(TEXT("MAX_PLAYERS"));
	inline const FName MaxRounds =	FName(TEXT("MAX_ROUNDS"));
}

USTRUCT(BlueprintType)
struct FPTWSessionConfig
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

USTRUCT(BlueprintType)
struct FAWSSessionConfig
{
	GENERATED_BODY()
	
	UPROPERTY()
	double CreationTime{};
	
	UPROPERTY()
	FString CreatorId{};
	
	UPROPERTY()
	int32 CurrentPlayerSessionCount{};
	
	UPROPERTY()
	FString DNSName{};
	
	UPROPERTY()
	FString FleetArn{};
	
	UPROPERTY()
	FString FleetId;
	
	UPROPERTY()
	TMap<FString, FString> GameProperties{};
	
	UPROPERTY()
	FString GameSessionData{};
	
	UPROPERTY()
	FString GameSessionId{};
	
	UPROPERTY()
	FString IPAddress{};
	
	UPROPERTY()
	FString Location{};
	
	UPROPERTY()
	FString MatchMakerData{};
	
	UPROPERTY()
	int32 MaximumPlayerSessionCount{};
	
	UPROPERTY()
	FString Name{};
	
	UPROPERTY()
	FString PlayerSessionCreationPolicy{};
	
	UPROPERTY()
	int32 Port{};
	
	UPROPERTY()
	FString Status{};
	
	UPROPERTY()
	FString StatusReason{};
	
	UPROPERTY()
	double TerminationTime{};
	
	void Dump() const;
};

