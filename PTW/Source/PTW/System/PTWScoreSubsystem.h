// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PTWScoreSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FPTWPlayerData
{
	GENERATED_BODY()
	
	FString PlayerName = "";
	int32 TotalPoints = 0;
	int32 Gold = 0;
	//TArray<AActor> InventoryItem;
	
};


UCLASS()
class PTW_API UPTWScoreSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	
	int32 CurrentGameRoundNumber = 0;
	TMap<int32, FPTWPlayerData> SavedPlayersData;
	
	void SavePlayersData(TArray<APlayerState> PlayerStates);

private:
	

	
};
