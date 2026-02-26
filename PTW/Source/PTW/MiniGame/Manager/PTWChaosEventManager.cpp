// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/Manager/PTWChaosEventManager.h"

#include "PTWChaosEventApply.h"
#include "MiniGame/Data/PTWChaosItemRow.h"



void UPTWChaosEventManager::InitChaosEventManager(APTWGameState* InGameState, const FPTWChaosEventRule& Rule)
{
	InitGameState(InGameState);
	InitChaosEventRule(Rule);
}

void UPTWChaosEventManager::BeginPlay()
{
	Super::BeginPlay();

	ChaosItemTable = LoadObject<UDataTable>(nullptr,
		TEXT("/Game/_PTW/Data/Event/Chaos/DT_ChaosItem.DT_ChaosItem"));
	
}

FName UPTWChaosEventManager::SelectRandomChaosItem()
{
	if (ChaosItemPool.Num() == 0 ) return NAME_None;

	// 카오스 아이템 총 개수 저장
	int32 TotalCount = 0;
	for (auto& Pair : ChaosItemPool)
	{
		TotalCount += Pair.Value;
	}
	
	int32 RandomValue = FMath::RandRange(0, TotalCount -1);
	int32 AccumulateValue = 0;
	
	// 랜덤으로 태그 선택 
	for (auto& Pair : ChaosItemPool)
	{
		AccumulateValue += Pair.Value;
		if (AccumulateValue > RandomValue)
		{
			return Pair.Key;
		}
	}

	return NAME_None;
}



void UPTWChaosEventManager::TriggerChaosEvent()
{
	FName RandChaosItemId = SelectRandomChaosItem();
	
	FPTWChaosItemRow* Row = ChaosItemTable->FindRow<FPTWChaosItemRow>(RandChaosItemId, "");
	if (!Row) return;

	UPTWChaosItemDefinition* Definition = Row->ChaosItemDefinition.LoadSynchronous();
	if (!Definition) return;

	UPTWChaosEventApply* ApplyEvent = NewObject<UPTWChaosEventApply>(this);
	if (!IsValid(ApplyEvent)) return;

	ApplyEvent->InitDefinition(Definition);
	ApplyEvent->ChaosEventApply(PTWGameState);
}

void UPTWChaosEventManager::AddChaosItemPool(FName ItemID)
{
	ChaosItemPool.FindOrAdd(ItemID)++;
}


void UPTWChaosEventManager::InitGameState(APTWGameState* InGameState)
{
	PTWGameState = InGameState;
}

void UPTWChaosEventManager::InitChaosEventRule(const FPTWChaosEventRule& Rule)
{
	ChaosEventRule = Rule;
}
