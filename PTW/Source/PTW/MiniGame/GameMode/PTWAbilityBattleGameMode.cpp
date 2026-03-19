// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/GameMode/PTWAbilityBattleGameMode.h"

#include "AbilitySystemComponent.h"
#include "HeadMountedDisplayTypes.h"
#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "Debug/PTWLogCategorys.h"
#include "GameFramework/SpectatorPawn.h"
#include "GAS/PTWAbilityBattleAttributeSet.h"
#include "GAS/PTWAbilitySystemComponent.h"
#include "Inventory/PTWInventoryComponent.h"
#include "MiniGame/ControllerComponent/AbilityBattle/PTWAbilityControllerComponent.h"
#include "MiniGame/Data/AbilityBattle/PTWAbilityRow.h"
#include "MiniGame/Manager/AbilityBattle/PTWRandomDraftSystem.h"
#include "MiniGame/PlayerStateComponent/PTWAbilityBattlePSComponent.h"
#include "UI/MiniGame/AbilityBattle/PTWAbilityDraftWidget.h"
#include "GameFramework/GameState.h"

APTWAbilityBattleGameMode::APTWAbilityBattleGameMode()
{
	
}

void APTWAbilityBattleGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	AttachPlayerStateComponent(NewPlayer);
}

void APTWAbilityBattleGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
	APTWPlayerController* PlayerController = Cast<APTWPlayerController>(C);
	if (!PlayerController) return;
	
	FInputModeUIOnly InputModeUIOnly;
	PlayerController->SetInputMode(InputModeUIOnly);
	PlayerController->bShowMouseCursor = true;
	PlayerController->FlushPressedKeys();

	Super::HandleSeamlessTravelPlayer(C);
}

void APTWAbilityBattleGameMode::StartGame()
{
	Super::StartGame();
	
	GrandAbilityBattleAttributeSet();
	InitAttributeSet();
	InitializeAbilityPool();

	StartDraft(1);

	for (APlayerState* PlayerState : PTWGameState->PlayerArray)
	{
		APTWPlayerState* PTWPlayerState = Cast<APTWPlayerState>(PlayerState);
		if (!PTWPlayerState) continue;

		 UPTWInventoryComponent* InventoryComponent = PTWPlayerState->GetInventoryComponent();
		if (!InventoryComponent) continue;

		InventoryComponent->SendEquipEventToASC(0);
	}
}

void APTWAbilityBattleGameMode::StartRound()
{
	Super::StartRound();

	EndDraft();
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

	TArray<FName> PoolCopy = *Pool;
	
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

		APTWPlayerState* PlayerState = PlayerController->GetPlayerState<APTWPlayerState>();
		if (!PlayerState) return;
		
		UPTWAbilityControllerComponent* AbilityControllerComponent =  Cast<UPTWAbilityControllerComponent>(PlayerController->GetControllerComponent());
		if (!AbilityControllerComponent) continue;

		UPTWAbilityBattlePSComponent* AbilityBattlePSComponent = Cast<UPTWAbilityBattlePSComponent>(PlayerState->GetMiniGameComponent());
		if (!AbilityBattlePSComponent) continue;
		
		TArray<FName> CurrentOptions = GenerateDraftOptions(Tier);

		AbilityBattlePSComponent->SetCurrentDraft(CurrentOptions);
		AbilityControllerComponent->Client_ShowDraftUI(CurrentOptions);
	}

	
}

void APTWAbilityBattleGameMode::EndDraft()
{
	AGameState* GS = Cast<AGameState>(GetWorld()->GetGameState());
	if (!GS) return;

	for (APlayerState* PlayerState : GS->PlayerArray)
	{
		APTWPlayerState* PTWPlayerState = Cast<APTWPlayerState>(PlayerState);
		if (!PTWPlayerState) continue;

		APTWPlayerController* PlayerController = Cast<APTWPlayerController>(PTWPlayerState->GetPlayerController());
		if (!PlayerController) continue;

		UPTWAbilityControllerComponent* AbilityControllerComponent = Cast<UPTWAbilityControllerComponent>(PlayerController->GetControllerComponent());
		if (!AbilityControllerComponent) continue;
		
		UPTWAbilityBattlePSComponent* PlayerStateComponent = Cast<UPTWAbilityBattlePSComponent>(PTWPlayerState->GetMiniGameComponent());
		if (!PlayerStateComponent) continue;

		if (!PlayerStateComponent->bFirstDraftCompleted)
		{
			const TArray<FName>& PlayerDrafts = PlayerStateComponent->GetCurrentDraft();
			if (PlayerDrafts.Num() == 0) continue;
			
			int32 RandIndex = FMath::RandRange(0, PlayerDrafts.Num() - 1);
			FName Draft = PlayerDrafts[RandIndex];
			AbilityControllerComponent->Server_SelectedAbility_Implementation(Draft);

			AbilityControllerComponent->Client_HideDraftUI();
		}

		AbilityControllerComponent->Client_GameInputMode();
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

void APTWAbilityBattleGameMode::AttachPlayerStateComponent(APlayerController* Controller)
{
	if (!Controller) return;

	APTWPlayerController* PlayerController = Cast<APTWPlayerController>(Controller);
	if (!PlayerController) return;

	APTWPlayerState* PlayerState = PlayerController->GetPlayerState<APTWPlayerState>();
	if (!PlayerState) return;
	
	if (UActorComponent* BeforeComponent = PlayerState->GetMiniGameComponent())
	{
		BeforeComponent->DestroyComponent();
	}
	
	UPTWAbilityBattlePSComponent* ActorComponent = NewObject<UPTWAbilityBattlePSComponent>(PlayerState, TEXT("MiniGameComponent"));
	if (!ActorComponent) return;

	ActorComponent->SetIsReplicated(true);
	PlayerState->AddInstanceComponent(ActorComponent);
	
	ActorComponent->RegisterComponent();

	PlayerState->SetMiniGameComponent(ActorComponent);
}


