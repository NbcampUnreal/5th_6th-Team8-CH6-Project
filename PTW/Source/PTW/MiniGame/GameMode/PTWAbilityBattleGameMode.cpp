// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/GameMode/PTWAbilityBattleGameMode.h"

#include "AbilitySystemComponent.h"
#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "Debug/PTWLogCategorys.h"
#include "GAS/PTWAbilityBattleAttributeSet.h"
#include "Inventory/PTWInventoryComponent.h"
#include "MiniGame/ControllerComponent/AbilityBattle/PTWAbilityControllerComponent.h"
#include "MiniGame/Data/AbilityBattle/PTWAbilityRow.h"
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
	Super::HandleSeamlessTravelPlayer(C);
	
	//UE_LOG(Log_AbilityBattle, Warning, TEXT("HandleSeamlessTravelPlayer"));
}

void APTWAbilityBattleGameMode::StartGame()
{
	Super::StartGame();
	
	GrandAbilityBattleAttributeSet();
	InitAttributeSet();
	InitializeAbilityPool();

	StartDraftAllPlayer(1);

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
	EndDraft();
	
	Super::StartRound();

	StartDraftChargeTimer();
}

void APTWAbilityBattleGameMode::RespawnPlayer(APTWPlayerController* SpawnPlayerController)
{
	APTWPlayerState* PlayerState = SpawnPlayerController->GetPlayerState<APTWPlayerState>();
	if (!PlayerState) return;

	UPTWAbilityBattlePSComponent* PSComponent = Cast<UPTWAbilityBattlePSComponent>(PlayerState->GetMiniGameComponent());
	if (!PSComponent) return;
	
	// 게임모드 스폰 관련 부분 함수로 만들고 함수 실행될 때 draftcharges == 0이면 플레이어 스폰 != 0 이면 return
	// 드래프트 선택 후 draftcharges가 0이면 플레이어 스폰 
	// 사망 중에는 draftcharges 게이지 안오르게 설정

	// 딜레이 후 UI 표시 해줘야함
	UPTWAbilityControllerComponent* ControllerComponent = Cast<UPTWAbilityControllerComponent>(SpawnPlayerController->GetControllerComponent());
	if (!ControllerComponent) return;

	if (PSComponent->DraftChargeCount > 0)
	{
		FTimerHandle DelayTimerHandle;

		GetWorld()->GetTimerManager().SetTimer(
			DelayTimerHandle,
			[ControllerComponent, this]()
			{
				if (!ControllerComponent) return;

				UE_LOG(Log_AbilityControllerComponent, Log, TEXT("3초 후 DraftUI 실행"));

				ControllerComponent->Client_ShowDraftUI(GenerateDraftOptions(1));
			},
			3.0f,
			false
		);

		UE_LOG(Log_AbilityControllerComponent, Log, TEXT("DraftUI 3초 딜레이 시작"));

		return;
	}
	
	//Super::RespawnPlayer(SpawnPlayerController);

	ControllerComponent->Client_RespawnPlayer(MiniGameRule.SpawnRule.bUseRespawn, MiniGameRule.SpawnRule.RespawnDelay);
	
	UE_LOG(Log_AbilityBattle, Warning, TEXT("RespawnPlayer"));
}

void APTWAbilityBattleGameMode::HandleRespawn(APTWPlayerController* PlayerController)
{
	//Super::HandleRespawn(PlayerController);

	APTWPlayerState* PlayerState = PlayerController->GetPlayerState<APTWPlayerState>();
	if (!PlayerState) return;

	UAbilitySystemComponent* ASC = PlayerState->GetAbilitySystemComponent();

	UPTWAbilityControllerComponent* ControllerComponent = Cast<UPTWAbilityControllerComponent>(PlayerController->GetControllerComponent());
	if (!ControllerComponent) return;
	
	UPTWAbilityBattlePSComponent* PSComponent = Cast<UPTWAbilityBattlePSComponent>(PlayerState->GetMiniGameComponent());
	if (!PSComponent) return;

	
	if (PSComponent->DraftChargeCount == 0)
	{
		Super::HandleRespawn(PlayerController); 
		
		ControllerComponent->Client_GameInputMode();
	}
	else
	{
		ControllerComponent->Client_ShowDraftUI(GenerateDraftOptions(1));
		return;
	}
	
	UPTWInventoryComponent* InventoryComponent = PlayerState->GetInventoryComponent();
	if (!InventoryComponent) return;
	
	InventoryComponent->SendEquipEventToASC(0);
	// 여기서 플레이어의 상태를 스폰 가능 상태로 만들고 드레프트 선택이 끝났을 때 스폰 가능 상태 + DraftCharges가 0일 경우 스폰 

	UE_LOG(Log_AbilityBattle, Warning, TEXT("=== After Respawn ==="));
	UE_LOG(Log_AbilityBattle, Warning, TEXT("Owner: %s"), *GetNameSafe(ASC->GetOwnerActor()));
	UE_LOG(Log_AbilityBattle, Warning, TEXT("Avatar: %s"), *GetNameSafe(ASC->GetAvatarActor()));
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
		
		ApplyEffect(ASC, InitAttributeEffectClass);
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
		int32 RandIndex = FMath::RandRange(0, PoolCopy.Num() - 1);
		
		FName RowId = (PoolCopy)[RandIndex];

		Result.Add(RowId);
		PoolCopy.RemoveAt(RandIndex);
	}

	UE_LOG(Log_AbilityBattle, Warning, TEXT("Draft Count %d"), Result.Num());
	
	return Result;
}

void APTWAbilityBattleGameMode::StartDraftAllPlayer(int32 Tier)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APTWPlayerController* PlayerController = Cast<APTWPlayerController>(It->Get());
		if (!PlayerController) continue;

		APTWPlayerState* PlayerState = PlayerController->GetPlayerState<APTWPlayerState>();
		if (!PlayerState) continue;
		
		UPTWAbilityControllerComponent* AbilityControllerComponent =  Cast<UPTWAbilityControllerComponent>(PlayerController->GetControllerComponent());
		if (!AbilityControllerComponent) continue;

		UPTWAbilityBattlePSComponent* AbilityBattlePSComponent = Cast<UPTWAbilityBattlePSComponent>(PlayerState->GetMiniGameComponent());
		if (!AbilityBattlePSComponent) continue;
		
		TArray<FName> CurrentOptions = GenerateDraftOptions(Tier);

		AbilityBattlePSComponent->SetCurrentDraft(CurrentOptions);
		AbilityControllerComponent->Client_ShowDraftUI(CurrentOptions);
	}
}

void APTWAbilityBattleGameMode::StartDraftChargeTimer()
{
	GetWorldTimerManager().SetTimer(DraftChargeTimerHandle, this, &APTWAbilityBattleGameMode::UpdateChargeTime, 0.1f, true);
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
			AbilityControllerComponent->Server_SelectedAbility(Draft);

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
		
		UPTWAbilityBattleAttributeSet* NewSet = NewObject<UPTWAbilityBattleAttributeSet>(PlayerState);
		if (!NewSet) continue;
		
		ASC->AddSpawnedAttribute(NewSet);
		
		ASC->GetGameplayAttributeValueChangeDelegate(UPTWAttributeSet::GetMaxHealthAttribute()).AddUObject(NewSet, &UPTWAbilityBattleAttributeSet::OnMaxHealthChanged);
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

void APTWAbilityBattleGameMode::AddDraftChargeAllPlayers()
{
	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		APTWPlayerState* PTWPlayerState = Cast<APTWPlayerState>(PlayerState);
		if (!PTWPlayerState) continue;

		UPTWAbilityBattlePSComponent* PSComponent = Cast<UPTWAbilityBattlePSComponent>(PTWPlayerState->GetMiniGameComponent());
		if (!PSComponent) continue;

		PSComponent->AddDraftCharges();
	}

	UE_LOG(Log_AbilityBattle, Log, TEXT("AddDraftChargeAllPlayers"));
}

void APTWAbilityBattleGameMode::UpdateChargeTime()
{
	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		APTWPlayerState* PTWPlayerState = Cast<APTWPlayerState>(PlayerState);
		if (!PTWPlayerState) continue;

		UPTWAbilityBattlePSComponent* PSComponent = Cast<UPTWAbilityBattlePSComponent>(PTWPlayerState->GetMiniGameComponent());
		if (!PSComponent) continue;
		
		PSComponent->UpdateChargeRemainTime(0.1f);
	}
}

void APTWAbilityBattleGameMode::ApplyEffect(UAbilitySystemComponent* ASC, TSubclassOf<UGameplayEffect> Effect)
{
	if (!Effect || !ASC) return;
	
	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(Effect, 1.f, Context);

	if (!Spec.IsValid()) return;

	//UE_LOG(Log_AbilityBattle, Warning, TEXT("ApplyInitAttributeSet"));
	ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
}


