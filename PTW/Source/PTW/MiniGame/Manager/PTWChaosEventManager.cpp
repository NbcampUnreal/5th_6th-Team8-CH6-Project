// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/Manager/PTWChaosEventManager.h"

#include "PTWChaosEventApply.h"

// Sets default values for this component's properties
void UPTWChaosEventManager::InitGameState(APTWGameState* InGameState)
{
	PTWGameState = InGameState;
}

UPTWChaosEventDefinition* UPTWChaosEventManager::SelectRandomChaosEvent()
{
	if (ChaosEventDefinitions.Num() ==0 ) return nullptr;
	
	const int32 RandInt = FMath::RandRange(0, ChaosEventDefinitions.Num());
	
	return ChaosEventDefinitions[RandInt]; 
}

void UPTWChaosEventManager::ApplyChaosEvent()
{
	UPTWChaosEventDefinition* ChaosEventDefinition = SelectRandomChaosEvent();
	if (!ChaosEventDefinition) return;

	UPTWChaosEventApply* ApplyEvent = NewObject<UPTWChaosEventApply>(this);
	if (!IsValid(ApplyEvent)) return;

	ApplyEvent->InitDefinition(ChaosEventDefinition);

	if (!PTWGameState) return;
	ApplyEvent->ChaosEventApply(PTWGameState);
}


