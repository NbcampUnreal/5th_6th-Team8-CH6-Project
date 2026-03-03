// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWDeliveryGameMode.h"


#include "AbilitySystemComponent.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "System/PTWItemSpawnManager.h"
#include "CoreFramework/PTWPlayerState.h"

APTWDeliveryGameMode::APTWDeliveryGameMode()
{
}

void APTWDeliveryGameMode::StartRound()
{
	SetMiniGameRule();
	Super::StartRound();
}

void APTWDeliveryGameMode::GiveDeliveryItems(APTWPlayerCharacter* TargetCharacter)
{
	ApplyMiniGameEffect(TargetCharacter);
	GivingDefaultWeapon(TargetCharacter);
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
	
	SpawnManager->SpawnSingleItem(TargetCharacter->GetPlayerState<APTWPlayerState>(), DefaultWeaponDef);
}

void APTWDeliveryGameMode::SetMiniGameRule()
{
	MiniGameRule.TimeRule.Timer = 180;
	MiniGameRule.KillRule.KillScore = 0;
	MiniGameRule.SpawnRule.RespawnDelay = 3.0f;
}
