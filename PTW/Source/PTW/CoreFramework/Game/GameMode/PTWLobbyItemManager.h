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
	void ApplyLobbyItem(APTWPlayerState* Buyer, const FName ItemId, APTWPlayerState* WinTarget = nullptr);
	void InitLobbyItemManager(UDataTable* DataTable, APTWGameState* GameState);

	void StartNewRound();
private:

	void InitLobbyItemTable(UDataTable* DataTable);
	void InitGameState(APTWGameState* GameState);
	
	UPROPERTY()
	TObjectPtr<UDataTable> LobbyItemTable;
	
	void HandleSavingGold(APTWPlayerState* Buyer, const FPTWLobbyItemRow* Row);

	UPROPERTY()
	TObjectPtr<APTWGameState> CachedGameState;
};
