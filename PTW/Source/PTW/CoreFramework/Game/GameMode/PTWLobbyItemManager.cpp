// Fill out your copyright notice in the Description page of Project Settings.


#include "CoreFramework/Game/GameMode/PTWLobbyItemManager.h"

#include "MiniGame/Data/PTWLobbyItemDefinition.h"
#include "MiniGame/Data/PTWLobbyItemRow.h"

void UPTWLobbyItemManager::InitLobbyItemTable(UDataTable* DataTable)
{
	LobbyItemTable = DataTable;
}

void UPTWLobbyItemManager::ApplyLobbyItem(APTWPlayerState* Buyer, const FName ItemId, APTWPlayerState* WinTarget)
{
	if (!Buyer) return;

	FPTWLobbyItemRow* Row = LobbyItemTable->FindRow<FPTWLobbyItemRow>(ItemId, TEXT(""));
	
	switch (Row->LobbyItemDefinition->ItemType)
	{
	case ELobbyItemType::Saving:
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


