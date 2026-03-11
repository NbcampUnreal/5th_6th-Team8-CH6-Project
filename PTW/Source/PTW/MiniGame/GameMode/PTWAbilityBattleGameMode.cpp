// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/GameMode/PTWAbilityBattleGameMode.h"

#include "AbilitySystemComponent.h"
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "Debug/PTWLogCategorys.h"
#include "GAS/PTWAbilityBattleAttributeSet.h"
#include "MiniGame/Manager/AbilityBattle/PTWRandomDraftSystem.h"


void APTWAbilityBattleGameMode::StartGame()
{
	Super::StartGame();
	
	GrandAbilityBattleAttributeSet();

	RandomDraftSystem = NewObject<UPTWRandomDraftSystem>(this);
	
}

void APTWAbilityBattleGameMode::GrandAbilityBattleAttributeSet()
{

	UE_LOG(Log_AbilityBattle, Warning, TEXT("GrandAbilityBattleAttribute"));
	
	for (APlayerState* PlayerState : PTWGameState->PlayerArray)
	{
		APTWPlayerState* PTWPlayerState = Cast<APTWPlayerState>(PlayerState);
		if (!PTWPlayerState) continue;

		UAbilitySystemComponent* ASC = PTWPlayerState->GetAbilitySystemComponent();
		if (!ASC) continue;
		
		UPTWAbilityBattleAttributeSet* NewASC = NewObject<UPTWAbilityBattleAttributeSet>(PlayerState);
		if (!NewASC) continue;
		
		ASC->AddSpawnedAttribute(NewASC);
	}
}
