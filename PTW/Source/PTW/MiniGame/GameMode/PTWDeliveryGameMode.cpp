// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWDeliveryGameMode.h"


#include "AbilitySystemComponent.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "System/PTWItemSpawnManager.h"
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "GAS/PTWDeliveryAttributeSet.h"

APTWDeliveryGameMode::APTWDeliveryGameMode()
{
}

void APTWDeliveryGameMode::StartRound()
{
	SetMiniGameRule();
	GrantDeliveryAttributeSet();
	Super::StartRound();
}

void APTWDeliveryGameMode::GiveDeliveryItems(APTWPlayerCharacter* TargetCharacter)
{
	ApplyMiniGameEffect(TargetCharacter);
	GivingDefaultWeapon(TargetCharacter);
}

void APTWDeliveryGameMode::GoalPlayer(APTWPlayerCharacter* TargetCharacter)
{
	if (GoalPlayers.Num() == 0)
	{
		StartCountDown();
	}
	
	GoalPlayers.Add(TargetCharacter);
}

void APTWDeliveryGameMode::HandlePlayerDeath(AActor* DeadActor, AActor* KillActor)
{
	IPTWCombatInterface* CombatInt = Cast<IPTWCombatInterface>(KillActor);
	if (!CombatInt) return;
	CombatInt->ApplyGameplayEffectToSelf(KillBonusEffect, 1.0f, FGameplayEffectContextHandle());
	Super::HandlePlayerDeath(DeadActor, KillActor);
}

void APTWDeliveryGameMode::ApplyMiniGameEffect(APTWPlayerCharacter* TargetCharacter)
{
	IPTWCombatInterface* CombatInterface = Cast<IPTWCombatInterface>(TargetCharacter);
	if (!CombatInterface) return;
	CombatInterface->ApplyGameplayEffectToSelf(DeliveryStartEffect, 1.0f, FGameplayEffectContextHandle());
}

void APTWDeliveryGameMode::GivingDefaultWeapon(APTWPlayerCharacter* TargetCharacter)
{
	UPTWItemSpawnManager* SpawnManager = GetWorld()->GetSubsystem<UPTWItemSpawnManager>();
	if (!SpawnManager) return;
	
	SpawnManager->SpawnSingleItem(TargetCharacter->GetPlayerState<APTWPlayerState>(), ItemDefinition);
}

void APTWDeliveryGameMode::SetMiniGameRule()
{
	MiniGameRule.TimeRule.Timer = 180;
	MiniGameRule.KillRule.KillScore = 0;
	MiniGameRule.SpawnRule.RespawnDelay = 1.5f;
}

void APTWDeliveryGameMode::GrantDeliveryAttributeSet()
{
	for (APlayerState* AS : PTWGameState->AlivePlayers)
	{
		APTWPlayerState* PS = Cast<APTWPlayerState>(AS);
		if (!PS) return;
		UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
		if (!ASC) return;
		
		if (ASC->GetAttributeSet(UPTWDeliveryAttributeSet::StaticClass())) return;
		
		UPTWDeliveryAttributeSet* NewSet = NewObject<UPTWDeliveryAttributeSet>(AS->GetPawn());
		ASC->AddSpawnedAttribute(NewSet);
		InitializeAttributeSet(ASC);
	}
}

void APTWDeliveryGameMode::InitializeAttributeSet(UAbilitySystemComponent* TargetASC)
{
	if (!TargetASC) return;
	TargetASC->SetNumericAttributeBase(UPTWDeliveryAttributeSet::GetBatteryLevelAttribute(), 1.0f);
}
