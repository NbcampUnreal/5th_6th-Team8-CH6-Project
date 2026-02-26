// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/Manager/PTWChaosEventManager.h"

#include "PTWChaosEventApply.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "MiniGame/Data/PTWChaosItemRow.h"
#include "System/PTWScoreSubsystem.h"


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

TPair<FName, int32>  UPTWChaosEventManager::SelectRandomChaosItem()
{
	if (ChaosItemPool.Num() == 0 )  return TPair<FName, int32>(NAME_None, 0);

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
			return TPair<FName, int32>(Pair.Key, Pair.Value); // 중첩 구현을 위해 id 개수도 함께 리턴
		}
	}

	return TPair<FName, int32>(NAME_None, 0);
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
	TPair<FName, int32> RandChaosItemId = SelectRandomChaosItem();
	if (RandChaosItemId.Key == NAME_None) return;
	
	FPTWChaosItemRow* Row = ChaosItemTable->FindRow<FPTWChaosItemRow>(RandChaosItemId.Key, "");
	if (!Row) return;

	UPTWChaosItemDefinition* Definition = Row->ChaosItemDefinition.LoadSynchronous();
	if (!Definition) return;

	CurrentApplyEvent = NewObject<UPTWChaosEventApply>(this, Definition->ChaosEventApplyClass);
	if (!IsValid(CurrentApplyEvent)) return;

	CurrentApplyEvent->InitDefinition(Definition);
	CurrentApplyEvent->SetStackCount(RandChaosItemId.Value);
	// 적용 딜레이 후 카오스 이벤트 적용
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
