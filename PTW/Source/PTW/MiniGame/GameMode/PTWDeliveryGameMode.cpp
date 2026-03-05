// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWDeliveryGameMode.h"


#include "AbilitySystemComponent.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "System/PTWItemSpawnManager.h"
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "GAS/PTWDeliveryAttributeSet.h"
#include "PTWGameplayTag/GameplayTags.h"

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
	if (!TargetCharacter) return;
	if (DeliveredCharacters.Contains(TargetCharacter)) return;
	
	ApplyMiniGameEffect(TargetCharacter);
	GivingDefaultWeapon(TargetCharacter);
	DeliveredCharacters.Add(TargetCharacter);
}

void APTWDeliveryGameMode::GoalPlayer(APTWPlayerCharacter* TargetCharacter)
{
	if (GoalPlayers.Num() == 0)
	{
		// 기존에 등록되었던 바인딩 함수 제거 후 새롭게 정의한 함수 등록
		// PTWGameState->OnCountDownFinished.Clear();
		// PTWGameState->OnCountDownFinished.AddDynamic(this, &APTWDeliveryGameMode::OnCountDownFinished);
		StartCountDown();
	}
	GoalPlayers.Add(TargetCharacter);
	
	ApplyGameEffect(TargetCharacter, InvincibleEffect);
}

void APTWDeliveryGameMode::HandlePlayerDeath(AActor* DeadActor, AActor* KillActor)
{
	APTWPlayerCharacter* TargetCharacter = Cast<APTWPlayerCharacter>(KillActor);
	ApplyGameEffect(TargetCharacter, KillBonusEffect);
	
	Super::HandlePlayerDeath(DeadActor, KillActor);
}

void APTWDeliveryGameMode::ApplyGameEffect(APTWPlayerCharacter* Target, TSubclassOf<UGameplayEffect> TargetGameplayEffect)
{
	IPTWCombatInterface* CombatInterface = Cast<IPTWCombatInterface>(Target);
	if (!CombatInterface) return;
	CombatInterface->ApplyGameplayEffectToSelf(TargetGameplayEffect, 1.0f, FGameplayEffectContextHandle());
}

void APTWDeliveryGameMode::OnCountDownFinished()
{
	if (GoalPlayers.Num() == 0)
	{
		Super::OnCountDownFinished();
	}
	else
	{
		EndRound();
	}
}

void APTWDeliveryGameMode::ApplyMiniGameEffect(APTWPlayerCharacter* TargetCharacter)
{
	ApplyGameEffect(TargetCharacter, DeliveryStartEffect);
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
