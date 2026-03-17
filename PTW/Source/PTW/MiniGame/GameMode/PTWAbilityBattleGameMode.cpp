// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/GameMode/PTWAbilityBattleGameMode.h"

#include "AbilitySystemComponent.h"
#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "Debug/PTWLogCategorys.h"
#include "GAS/PTWAbilityBattleAttributeSet.h"
#include "MiniGame/ControllerComponent/AbilityBattle/PTWAbilityControllerComponent.h"
#include "MiniGame/Data/AbilityBattle/PTWAbilityRow.h"
#include "MiniGame/Manager/AbilityBattle/PTWRandomDraftSystem.h"


void APTWAbilityBattleGameMode::StartGame()
{
	Super::StartGame();
	
	GrandAbilityBattleAttributeSet();
	InitAttributeSet();
	InitializeAbilityPool();

	StartDraft(1);
}

void APTWAbilityBattleGameMode::InitAttributeSet()
{
	if (!InitAttributeEffectClass) return;

	UE_LOG(Log_AbilityBattle, Warning, TEXT("InitAttributeSet"));
	
	for (APlayerState* PlayerState : PTWGameState->PlayerArray)
	{
		APTWPlayerState* PTWPlayerState = Cast<APTWPlayerState>(PlayerState);
		if (!PTWPlayerState) continue;

		UAbilitySystemComponent* ASC = PTWPlayerState->GetAbilitySystemComponent();
		if (!ASC) continue;
		
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(InitAttributeEffectClass, 1.f, Context);

		if (!Spec.IsValid()) continue;

		UE_LOG(Log_AbilityBattle, Warning, TEXT("ApplyInitAttributeSet"));
		ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}
}

void APTWAbilityBattleGameMode::InitializeAbilityPool()
{
	if (!AbilityDataTable) return;
	
	for (auto& Row : AbilityDataTable->GetRowMap())
	{
		FName RowName = Row.Key;
		FPTWAbilityRow* Data = (FPTWAbilityRow*)Row.Value;
		if (!Data) continue;

		if (Data->AbilityDefinition.IsNull()) continue;
        
		TierAbilityPool.FindOrAdd(Data->Tier).Add(RowName);
	}
}

TArray<FName> APTWAbilityBattleGameMode::GenerateDraftOptions(int32 Tier)
{
	TArray<FName> Result;

	TArray<FName>* Pool = TierAbilityPool.Find(Tier);
	if (!Pool)
	{
		UE_LOG(Log_AbilityBattle, Warning, TEXT("Pool is nullptr"));
		return Result;
	}
	
	for (int i = 0; i < DraftOptionCount; i++)
	{
		int32 RandIndex = FMath::RandRange(0, Pool->Num() - 1);
		
		FName RowId = (*Pool)[RandIndex];

		Result.Add(RowId);
		Pool->RemoveAt(RandIndex);
	}

	UE_LOG(Log_AbilityBattle, Warning, TEXT("Draft Count %d"), Result.Num());
	
	return Result;
}

void APTWAbilityBattleGameMode::StartDraft(int32 Tier)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APTWPlayerController* PlayerController = Cast<APTWPlayerController>(It->Get());
		if (!PlayerController) continue;

		UPTWAbilityControllerComponent* AbilityControllerComponent =  Cast<UPTWAbilityControllerComponent>(PlayerController->GetControllerComponent());
		if (!AbilityControllerComponent) continue;

		AbilityControllerComponent->Client_ShowDraftUI(GenerateDraftOptions(Tier));
	}
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




