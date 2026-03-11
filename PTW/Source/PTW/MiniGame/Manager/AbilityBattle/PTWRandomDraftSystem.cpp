// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/Manager/AbilityBattle/PTWRandomDraftSystem.h"

void UPTWRandomDraftSystem::InitRandomDraftSystem(APTWGameState* InGameState, UDataTable* InDataTable)
{
	InitAbilityDataTable(InDataTable);
	InitGameState(InGameState);
}

void UPTWRandomDraftSystem::InitAbilityDataTable(UDataTable* InDataTable)
{
	AbilityDataTable = InDataTable;
}

void UPTWRandomDraftSystem::InitGameState(APTWGameState* InGameState)
{
	GameState = InGameState;
}


