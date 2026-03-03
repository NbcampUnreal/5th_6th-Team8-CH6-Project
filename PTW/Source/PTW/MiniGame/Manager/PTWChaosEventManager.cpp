// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/Manager/PTWChaosEventManager.h"

#include "PTWChaosEventApply.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "MiniGame/Data/PTWChaosItemRow.h"


void UPTWChaosEventManager::InitChaosEventManager(APTWGameState* InGameState, const FPTWChaosEventRule& Rule, const TArray<FPTWChaosItemEntry>& Entries)
{
	InitGameState(InGameState);
	InitChaosEventRule(Rule);
	InitChaosItemEntries(Entries);
}

void UPTWChaosEventManager::InitChaosItemEntries(const TArray<FPTWChaosItemEntry>& Entries)
{
	ChaosItemEntries = Entries;
}

void UPTWChaosEventManager::BeginPlay()
{
	Super::BeginPlay();

	ChaosItemTable = LoadObject<UDataTable>(nullptr,
		TEXT("/Game/_PTW/Data/Event/Chaos/DT_ChaosItem.DT_ChaosItem"));
	
}

FPTWChaosItemEntry UPTWChaosEventManager::SelectRandomChaosItem()
{
	if (ChaosItemEntries.Num() == 0) return FPTWChaosItemEntry{};

	int32 RandomIndex = FMath::RandRange(0, ChaosItemEntries.Num() - 1);
	return ChaosItemEntries[RandomIndex];
}

void UPTWChaosEventManager::StartChaosEvent()
{
	if (IsValid(CurrentApplyEvent)) return;
	
	switch (ChaosEventRule.RandomEventType)
	{
	case EPTWRandomEventType::Interval:
		if (!GetWorld()->GetTimerManager().IsTimerActive(ChaosEventTimerHandle))
		{
			GetWorld()->GetTimerManager().SetTimer(
				ChaosEventTimerHandle, this,
				&UPTWChaosEventManager::TriggerChaosEvent,
				ChaosEventRule.IntervalTime, false);
		}
		break;

	case EPTWRandomEventType::TimeRemain:
		if (PTWGameState->GetRemainTime() <= ChaosEventRule.RemainTimeThreshold)
		{
			TriggerChaosEvent();
		}
		break;

	case EPTWRandomEventType::SurvivalThreshold:
		if (PTWGameState && ChaosEventRule.MinSurvivorCount >= PTWGameState->AlivePlayers.Num())
		{
			TriggerChaosEvent();
		}
		break;

	default:
		break;
	}
}

void UPTWChaosEventManager::TriggerChaosEvent()
{
	FPTWChaosItemEntry SelectedEntry = SelectRandomChaosItem();
	if (SelectedEntry.ItemId == NAME_None) return;
    
	FPTWChaosItemRow* Row = ChaosItemTable->FindRow<FPTWChaosItemRow>(SelectedEntry.ItemId, "");
	if (!Row) return;

	UPTWChaosItemDefinition* Definition = Row->ChaosItemDefinition.LoadSynchronous();
	if (!Definition) return;

	CurrentApplyEvent = NewObject<UPTWChaosEventApply>(this);
	if (!IsValid(CurrentApplyEvent)) return;

	CurrentApplyEvent->InitDefinition(Definition);

	//UI 데이터 전달
	FPTWChaosEventUIData UIData;
	UIData.BuyerName = SelectedEntry.PlayerName;
	UIData.ItemDisplayName = Definition->DisplayName;
	UIData.ItemIcon = Definition->Icon;
	
	// 동일 ItemId 개수만큼 스택
	int32 StackCount = 0;
	for (const FPTWChaosItemEntry& Entry : ChaosItemEntries)
	{
		if (Entry.ItemId == SelectedEntry.ItemId)
		{
			StackCount++;
		}
	}
	CurrentApplyEvent->SetStackCount(StackCount);

	// 중복 아이템 삭제
	ChaosItemEntries.RemoveAll([&SelectedEntry](const FPTWChaosItemEntry& Entry)
	  {
		  return Entry.ItemId == SelectedEntry.ItemId;
	  });
	
	// 적용 딜레이 후 카오스 이벤트 실행
	GetWorld()->GetTimerManager().SetTimer(ChaosEventApplyDelayHandle, [this]()
	{
		if (!IsValid(CurrentApplyEvent)) return;
		CurrentApplyEvent->ApplyChaosEvent(PTWGameState);

		GetWorld()->GetTimerManager().SetTimer(ChaosEventDurationHandle, [this]()
		{
			EndChaosEvent();

		}, ChaosEventRule.ApplyDuration, false);

	}, ChaosEventRule.ApplyDelayTime, false);
}


void UPTWChaosEventManager::EndChaosEvent()
{
	ClearAllTimer();
	
	if (!IsValid(CurrentApplyEvent)) return;
	CurrentApplyEvent->ChaosEventEnd();
	CurrentApplyEvent = nullptr;

	if (ChaosEventRule.RandomEventType == EPTWRandomEventType::Interval)
	{
		StartChaosEvent();
	}
}

void UPTWChaosEventManager::ClearAllTimer()
{
	GetWorld()->GetTimerManager().ClearTimer(ChaosEventTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(ChaosEventApplyDelayHandle);
	GetWorld()->GetTimerManager().ClearTimer(ChaosEventDurationHandle);
}

void UPTWChaosEventManager::InitGameState(APTWGameState* InGameState)
{
	PTWGameState = InGameState;
}

void UPTWChaosEventManager::InitChaosEventRule(const FPTWChaosEventRule& Rule)
{
	ChaosEventRule = Rule;
}
