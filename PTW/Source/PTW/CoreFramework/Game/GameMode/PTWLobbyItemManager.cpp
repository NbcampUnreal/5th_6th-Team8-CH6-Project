// Fill out your copyright notice in the Description page of Project Settings.


#include "CoreFramework/Game/GameMode/PTWLobbyItemManager.h"

#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "MiniGame/Data/PTWLobbyItemDefinition.h"
#include "MiniGame/Data/PTWLobbyItemRow.h"

void UPTWLobbyItemManager::InitLobbyItemManager(UDataTable* DataTable, APTWGameState* GameState)
{
	InitLobbyItemTable(DataTable);
	InitGameState(GameState);
}

void UPTWLobbyItemManager::StartNewRound()
{
	
}

void UPTWLobbyItemManager::InitLobbyItemTable(UDataTable* DataTable)
{
	LobbyItemTable = DataTable;
}

void UPTWLobbyItemManager::InitGameState(APTWGameState* GameState)
{
	CachedGameState = GameState;
}

void UPTWLobbyItemManager::ApplyLobbyItem(APTWPlayerState* Buyer, const FName ItemId, APTWPlayerState* WinTarget)
{
	if (!Buyer) return;

	FPTWLobbyItemRow* Row = LobbyItemTable->FindRow<FPTWLobbyItemRow>(ItemId, TEXT(""));
	
	switch (Row->LobbyItemDefinition->ItemType)
	{
	case ELobbyItemType::SavingGold:
		HandleSavingGold(Buyer, Row);
		break;
	case ELobbyItemType::PredictionWin:
		break;
	case ELobbyItemType::GambleBox:
		break;
	case ELobbyItemType::RandomActive:
		break;
	case ELobbyItemType::RandomPassive:
		break;
	}
}


void UPTWLobbyItemManager::HandleSavingGold(APTWPlayerState* Buyer, const FPTWLobbyItemRow* Row)
{
	if (!CachedGameState) return;
	
	FSavingData SavingGold;
	SavingGold.TargetRound = Row->LobbyItemDefinition->DelayRound + CachedGameState->GetCurrentRound();
	SavingGold.RewardAmount = Row->LobbyItemDefinition->RewardAmount;

	FPTWLobbyItemData LobbyItemData = Buyer->GetLobbyItemData();
	LobbyItemData.SavingData.Add(SavingGold);
	Buyer->SetLobbyItemData(LobbyItemData);
}

