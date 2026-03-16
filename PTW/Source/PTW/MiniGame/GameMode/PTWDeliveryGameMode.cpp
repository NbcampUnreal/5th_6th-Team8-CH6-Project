// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWDeliveryGameMode.h"


#include "AbilitySystemComponent.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/PTWPlayerController.h"
#include "System/PTWItemSpawnManager.h"
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "GAS/PTWDeliveryAttributeSet.h"
#include "MiniGame/ControllerComponent/Delivery/PTWDeliveryControllerComponent.h"
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

void APTWDeliveryGameMode::GiveDeliveryItems(APTWPlayerCharacter* TargetCharacter, TSubclassOf<UGameplayEffect> EffectToApply)
{
	if (!TargetCharacter) return;
	if (DeliveredCharacters.Contains(TargetCharacter)) return;
	
	ApplyGameEffect(TargetCharacter, EffectToApply);
	GivingDefaultWeapon(TargetCharacter);
	DeliveryUISetting(TargetCharacter);
	DeliveredCharacters.Add(TargetCharacter);
}

void APTWDeliveryGameMode::GoalPlayer(APTWPlayerCharacter* TargetCharacter, TSubclassOf<UGameplayEffect> EffectToApply)
{
	if (GoalPlayers.Num() == 0)
	{
		StartEndCountDown();
	}
	GoalPlayers.Add(TargetCharacter);
	
	ApplyGameEffect(TargetCharacter, EffectToApply);
}

void APTWDeliveryGameMode::SetPlayerSpawnLocation(APTWPlayerController* PC, FVector NewLocation)
{
	PlayerSpawnPoints.FindOrAdd(PC) = NewLocation;
}

FTransform APTWDeliveryGameMode::GetPlayerSpawnTransform(APTWPlayerController* PC)
{
	if (FVector* SpawnLoc = PlayerSpawnPoints.Find(PC))
	{
		return FTransform(FRotator::ZeroRotator, *SpawnLoc);
	}
	
	return FTransform();
}

void APTWDeliveryGameMode::HandlePlayerDeath(AActor* DeadActor, AActor* KillActor)
{
	APTWPlayerCharacter* TargetCharacter = Cast<APTWPlayerCharacter>(KillActor);
	ApplyGameEffect(TargetCharacter, KillBonusEffect);
	
	APTWPlayerCharacter* DeadCharacter = Cast<APTWPlayerCharacter>(DeadActor);
	
	IPTWCombatInterface* DeadCombatInterface = CastToPTWCombatInterface(DeadCharacter);
	if (!DeadCombatInterface) return;
	
	DeadCombatInterface->RemoveEffectWithTag(GameplayTags::MiniGame::Delivery);
	
	Super::HandlePlayerDeath(DeadActor, KillActor);
}

void APTWDeliveryGameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);
	
	APTWPlayerCharacter* TargetCharacter = Cast<APTWPlayerCharacter>(NewPlayer->GetPawn());
	if (!TargetCharacter) return;
	
	if (CheckingDeadPlayer(NewPlayer))
	{
		IPTWCombatInterface* CombatInterface = CastToPTWCombatInterface(TargetCharacter);
		if (!CombatInterface) return;
		CombatInterface->ApplyGameplayEffectToSelf(RestartPlayerEffect, 1.0f, FGameplayEffectContextHandle());
	}
}

void APTWDeliveryGameMode::ApplyGameEffect(APTWPlayerCharacter* Target, TSubclassOf<UGameplayEffect> TargetGameplayEffect)
{
	IPTWCombatInterface* CombatInterface = CastToPTWCombatInterface(Target);
	if (!CombatInterface) return;
	CombatInterface->ApplyGameplayEffectToSelf(TargetGameplayEffect, 1.0f, FGameplayEffectContextHandle());
}

void APTWDeliveryGameMode::StartEndCountDown()
{
	DeliveryComp->ShowCountDownWidget();
	UpdateCountDown();
	GetWorld()->GetTimerManager().SetTimer(CountDownTimerHandle, this, &APTWDeliveryGameMode::UpdateCountDown,1.0f,true);
}

void APTWDeliveryGameMode::UpdateCountDown()
{
	if (FinalCount == 0)
	{
		StopCountDown();
	}
		
	DeliveryComp->SetCountDownText(FinalCount);
	FinalCount--;
}

void APTWDeliveryGameMode::StopCountDown()
{
	GetWorld()->GetTimerManager().ClearTimer(CountDownTimerHandle);
	
	FTimerHandle EndTimerWaitHandle;
	GetWorld()->GetTimerManager().SetTimer(EndTimerWaitHandle, this, &APTWDeliveryGameMode::EndTimer, 3.0f, false);
}

bool APTWDeliveryGameMode::CheckingDeadPlayer(AController* NewPlayer)
{
	APTWPlayerState* PTWPlayerState = NewPlayer->GetPlayerState<APTWPlayerState>();
	check(PTWPlayerState);
	
	return PTWPlayerState->GetPlayerRoundData().DeathCount != 0;
}

IPTWCombatInterface* APTWDeliveryGameMode::CastToPTWCombatInterface(APTWPlayerCharacter* PlayerCharacter)
{
	IPTWCombatInterface* PTWCombatInterface = Cast<IPTWCombatInterface>(PlayerCharacter);
	return PTWCombatInterface;
}

void APTWDeliveryGameMode::GivingDefaultWeapon(APTWPlayerCharacter* TargetCharacter)
{
	UPTWItemSpawnManager* SpawnManager = GetWorld()->GetSubsystem<UPTWItemSpawnManager>();
	if (!SpawnManager) return;
	
	SpawnManager->SpawnSingleItem(TargetCharacter->GetPlayerState<APTWPlayerState>(), DeliveryDefaultWeapon);
}

void APTWDeliveryGameMode::SetMiniGameRule()
{
	MiniGameRule.TimeRule.Timer = 180;
	MiniGameRule.KillRule.KillScore = 0;
	MiniGameRule.SpawnRule.RespawnDelay = 5.0f;
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
	TargetASC->SetNumericAttributeBase(UPTWDeliveryAttributeSet::GetMaxBatteryLevelAttribute(), 1.0f);
	float MaxValue = TargetASC->GetNumericAttribute(UPTWDeliveryAttributeSet::GetMaxBatteryLevelAttribute());
	TargetASC->SetNumericAttributeBase(UPTWDeliveryAttributeSet::GetBatteryLevelAttribute(), MaxValue);
}

void APTWDeliveryGameMode::DeliveryUISetting(APTWPlayerCharacter* TargetCharacter)
{
	if (APTWPlayerController* PlayerController = Cast<APTWPlayerController>(TargetCharacter->GetController()))
	{
		DeliveryComp = Cast<UPTWDeliveryControllerComponent>(PlayerController->GetComponentByClass(UPTWDeliveryControllerComponent::StaticClass()));
		
		if (DeliveryComp)
		{
			DeliveryComp->AddBatteryUI();
		}
	}
}
