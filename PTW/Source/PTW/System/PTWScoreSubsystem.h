// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PTWScoreSubsystem.generated.h"



UCLASS()
class PTW_API UPTWScoreSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//TMap<int32, FPTWPlayerData> SavedPlayersData;
	
	void SavePlayersData();
	void SaveCurrentGameRound(int32 NewGameRound);
	
	FORCEINLINE int32 GetCurrentGameRound() const {return CurrentGameRound;}
private:
	int32 CurrentGameRound = 1;

	
};
