// Fill out your copyright notice in the Description page of Project Settings.


#include "CoreFramework/Game/GameMode/PTWLobbyItemManager.h"

#include "PTWGameMode.h"
#include "PTWLobbyGameMode.h"
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

int32 UPTWLobbyItemManager::TakeSavingsReward(APTWPlayerState* PlayerState)
{
	int32 SavingGold = 0;
	
	// 적금 아이템이 있을 경우 적금 골드를 받을 수 있는 지 확인하고 골드 추가
	if (!PlayerState->GetLobbyItemData().SavingData.IsEmpty())
	{
		TArray<FSavingData>& SavingDataArray = PlayerState->GetLobbyItemData().SavingData;
		
		for (int32 i = SavingDataArray.Num() - 1; i >= 0; i--)
		{
			if (SavingDataArray[i].TargetRound == CachedGameState->GetCurrentRound())
			{
				SavingGold += SavingDataArray[i].RewardAmount;
				SavingDataArray.RemoveAt(i);
			}
		}
	}

	return SavingGold;
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
	if (!Row) return;
	UPTWLobbyItemDefinition* LobbyItemDefinition = Row->LobbyItemDefinition;
	if (!LobbyItemDefinition) return;
	
	switch (LobbyItemDefinition->ItemType)
	{
	case ELobbyItemType::SavingGold:
		HandleSavingGold(Buyer, LobbyItemDefinition);
		break;
	case ELobbyItemType::PredictionWin:
		break;
	case ELobbyItemType::GambleBox:
		HandleGambleBox(Buyer, LobbyItemDefinition);
		break;
	case ELobbyItemType::RandomActive:
		break;
	case ELobbyItemType::RandomPassive:
		break;
	}
}

void UPTWLobbyItemManager::HandleSavingGold(APTWPlayerState* Buyer, const UPTWLobbyItemDefinition* LobbyItemDefinition)
{
	if (!CachedGameState) return;
	if (!Buyer || !LobbyItemDefinition) return;
	
	FSavingData SavingGold;
	SavingGold.TargetRound = LobbyItemDefinition->DelayRound + CachedGameState->GetCurrentRound();
	SavingGold.RewardAmount = LobbyItemDefinition->RewardAmount;

	FPTWLobbyItemData LobbyItemData = Buyer->GetLobbyItemData();
	LobbyItemData.SavingData.Add(SavingGold);
	Buyer->SetLobbyItemData(LobbyItemData);
}

void UPTWLobbyItemManager::HandleGambleBox(APTWPlayerState* Buyer, const UPTWLobbyItemDefinition* LobbyItemDefinition)
{
	if (!Buyer || !LobbyItemDefinition) return;

	UObject* Owner = GetOuter();
	if (!Owner) return;
	
	APTWLobbyGameMode* LobbyGameMode = Cast<APTWLobbyGameMode>(Owner);
	if (!LobbyGameMode) return;

	int32 RandGold = FMath::RandRange(1, LobbyItemDefinition->GambleBoxMaxAmount);
	
	LobbyGameMode->AddGold(Buyer, RandGold);
	
}


