// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MiniGame/Data/PTWLobbyItemRow.h"
#include "UObject/Object.h"
#include "PTWLobbyItemManager.generated.h"

class APTWGameState;
class APTWPlayerState;
/**
 * 
 */
UCLASS()
class PTW_API UPTWLobbyItemManager : public UObject
{
	GENERATED_BODY()

public:
	//* 로비 아이템 구매 시 호출 함수 */
	void ApplyLobbyItem(APTWPlayerState* Buyer, const FName ItemId);
	void InitLobbyItemManager(UDataTable* DataTable, APTWGameState* GameState);

	void StartNewRound();
	
	int32 TakeSavingsReward(APTWPlayerState* PlayerState);
	int32 TakePredictionWinReward(APTWPlayerState* PlayerState);

	int32 TakeGoldReward(APTWPlayerState* PlayerState);
private:

	void InitLobbyItemTable(UDataTable* DataTable);
	void InitGameState(APTWGameState* GameState);
	
	void HandleSavingGold(APTWPlayerState* Buyer, const UPTWLobbyItemDefinition* LobbyItemDefinition);
	void HandleGambleBox(APTWPlayerState* Buyer, const UPTWLobbyItemDefinition* LobbyItemDefinition);
	void HandlePredictionWin(APTWPlayerState* Buyer, const UPTWLobbyItemDefinition* LobbyItemDefinition);

	void AddGold(APTWPlayerState* Buyer, int32 Gold);
	UPROPERTY()
	TObjectPtr<APTWGameState> CachedGameState;

	UPROPERTY()
	TObjectPtr<UDataTable> LobbyItemTable;

	int32 PredictionWinReward = 0;
};
