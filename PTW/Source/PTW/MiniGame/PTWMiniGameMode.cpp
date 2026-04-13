// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWMiniGameMode.h"

#include "PTW/System/PTWItemSpawnManager.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "System/Prop/PTWPropSubsystem.h"
#include "GAS/PTWAttributeSet.h"
#include "EngineUtils.h"
#include "Manager/PTWChaosEventManager.h"
#include "PTW/Inventory/PTWItemDefinition.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h"
#include "Engine/TargetPoint.h"
#include "AIController.h"
#include "Component/PTWResultSequenceComponent.h"
#include "Component/PTWSpawnComponent.h"
#include "Component/PTWWinConditionComponent.h"
#include "ControllerComponent/PTWBaseControllerComponent.h"
#include "ControllerComponent/AbilityBattle/PTWAbilityControllerComponent.h"
#include "CoreFramework/Character/Component/PTWWeaponComponent.h"
#include "Debug/PTWLogCategorys.h"
#include "Gameplay/Actor/PTWResultCharacter.h"
#include "Inventory/PTWInventoryComponent.h"
#include "Inventory/Instance/PTWItemInstance.h"
#include "Inventory/Instance/PTWWeaponInstance.h"
#include "MiniGame/PTWMiniGameMapRow.h"

class UPTWScoreSubsystem;

#pragma region public

APTWMiniGameMode::APTWMiniGameMode()
{
	ChaosEventManager = CreateDefaultSubobject<UPTWChaosEventManager>(TEXT("ChaosEventManager"));
	WinConditionComponent = CreateDefaultSubobject<UPTWWinConditionComponent>(TEXT("WinConditionComponent"));
	SpawnComponent  = CreateDefaultSubobject<UPTWSpawnComponent>(TEXT("SpawnComponent"));
	ResultSequenceComponent = CreateDefaultSubobject<UPTWResultSequenceComponent>(TEXT("ResultSequenceComponent"));
	
	bIsGameEnded = false;
}

void APTWMiniGameMode::AddWinPoint(AActor* Actor, int32 AddPoint)
{
	
}

bool APTWMiniGameMode::PlayerDeadCheck(AController* Controller)
{
	if (PendingRespawnItems.Contains(Controller))
	{
		return true;
	}
	
	return false;
}

FItemArrayWrapper APTWMiniGameMode::GetOldPlayerItems(AController* Controller) const
{
	if (PendingRespawnItems.Contains(Controller))
	{
		return PendingRespawnItems[Controller];
	}
	else
	{
		return FItemArrayWrapper();
	}
}

void APTWMiniGameMode::HandlePlayerDeath(AActor* DeadActor, AActor* KillActor)
{
	if (!IsValid(DeadActor)) return;
	
	APTWPlayerController* DeadPlayerController = nullptr;
	APlayerState* DeadPlayerState = nullptr;
	
	if (const APawn* DeadPawn = Cast<APawn>(DeadActor))
	{
		DeadPlayerController = DeadPawn->GetController<APTWPlayerController>();
		DeadPlayerState = DeadPawn->GetPlayerState();

		APTWPlayerState* PS = Cast<APTWPlayerState>(DeadPlayerState);
		if (!PS || !DeadPawn) return;

		UPTWInventoryComponent* InvenComp = PS->GetInventoryComponent();
		if (!InvenComp) return;
		
		DeathPlayerWeaponHandler(InvenComp);
		
		APTWPlayerCharacter* PC = Cast<APTWPlayerCharacter>(DeadActor);
		if (!PC) return;
		PlayerMontageStop(PC);
		
	}

	APlayerState* KillPlayerState = nullptr;
	if (IsValid(KillActor))
	{
		if (APawn* KillPawn = Cast<APawn>(KillActor))
		{
			KillPlayerState = KillPawn->GetPlayerState<APlayerState>();
		}
		else
		{
			KillPlayerState = Cast<APlayerState>(KillActor);
		}
	}
	
	AddKillDeathCount(DeadPlayerState, KillPlayerState);
	
	if (!PTWGameState) return;
	PTWGameState->UpdateRanking(MiniGameRule);
	PTWGameState->AlivePlayers.Remove(DeadPlayerState);

	StartChaosEvent();
	
	CheckEndGameCondition();
	
	RespawnPlayer(DeadPlayerController);
}
#pragma endregion public

#pragma region protected

#pragma region override

void APTWMiniGameMode::InitGameState()
{
	Super::InitGameState();

	TravelLevelName = TEXT("/Game/_PTW/Maps/Lobby");

	if (PTWGameState)
	{
		PTWGameState->SetCurrentPhase(EPTWGamePhase::Loading);
		PTWGameState->SetMaxMiniGameRound(MiniGameRule.TimeRule.Round);
	}
}

void APTWMiniGameMode::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (!PTWGameState) return;
	
	if (!ChaosEventManager) return;
	ChaosEventManager->InitChaosEventManager(PTWGameState, MiniGameRule.ChaosEventRule, CachedGameData.ChaosItemEntries);
	
}

void APTWMiniGameMode::BeginPlay()
{
	Super::BeginPlay();

	
	if (!PTWGameState) return;
	

#if WITH_EDITOR
	// PIE에서는 딜레이 후 강제 시작
	FTimerHandle PIEStartTimer;
	GetWorldTimerManager().SetTimer(PIEStartTimer, this, &APTWMiniGameMode::StartGame, 2.f, false);
#endif

	// 플레이어 로딩 완료 체크에서 문제가 생겨 미작동 시 10초 후 게임 강제 시작
	FTimerHandle StartGameTimer;
	GetWorldTimerManager().SetTimer(StartGameTimer, FTimerDelegate::CreateLambda([this]()
	{
		if (!bIsGameStarted)
		{
			StartGame();
		}
		
	}), 10.f, false);
	
	APTWGameState* GS = GetGameState<APTWGameState>();
	if (!GS || !MiniGameMapTable) return;

	UWorld* World = GetWorld();
	if (!World) return;

	const FString CurrentMapPath = World->RemovePIEPrefix(World->GetPathName());

	FPTWRouletteData Data = GS->GetRouletteData();

	for (const auto& Pair : MiniGameMapTable->GetRowMap())
	{
		FName RowName = Pair.Key;
		const FPTWMiniGameMapRow* Row = reinterpret_cast<FPTWMiniGameMapRow*>(Pair.Value);

		if (!Row) continue;

		if (Row->Map.ToSoftObjectPath().GetAssetPathString() == CurrentMapPath)
		{
			Data.MapRowName = RowName;
			GS->SetRouletteData(Data);
			return;
		}
	}
}

void APTWMiniGameMode::Logout(AController* Exiting)
{
	APlayerState* PlayerState = Exiting->GetPlayerState<APlayerState>();
	Super::Logout(Exiting);

	if (!PTWGameState || !PlayerState) return;
	
	PTWGameState->AlivePlayers.Remove(PlayerState);
}

void APTWMiniGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	
	if (!IsValid(PTWGameState)) return;
	if (!NewPlayer) return;
	
	APTWPlayerState* PlayerState = NewPlayer->GetPlayerState<APTWPlayerState>();
	if (!IsValid(PlayerState)) return;
	
	//SetInputBlock(true);
	
	PTWGameState->AddRankedPlayer(PlayerState);
	
	AttachControllerComponent(NewPlayer);
}
void APTWMiniGameMode::RestartPlayer(AController* NewPlayer)
{
	if (!NewPlayer) return;
	
	Super::RestartPlayer(NewPlayer);
	
	if (!IsValid(PTWGameState)) return;
	
	APlayerState* PlayerState = NewPlayer->GetPlayerState<APlayerState>();
	if (!PlayerState) return;
	
	PTWGameState->AlivePlayers.Add(PlayerState);

	if (NewPlayer->IsA<AAIController>())
	{
		if (APTWPlayerState* PTWPlayerState = Cast<APTWPlayerState>(PlayerState))
		{
			PTWGameState->AddRankedPlayer(PTWPlayerState);
		}
	}
	
	ApplyMiniGameTag(NewPlayer);
	RemoveTags(NewPlayer);
	InitPlayerHealth(NewPlayer);
	
	
	if (APTWBaseCharacter* BaseCharacter = Cast<APTWBaseCharacter>(NewPlayer->GetPawn()))
	{
		BaseCharacter->OnCharacterDied.RemoveDynamic(this, &APTWMiniGameMode::HandlePlayerDeath);
		BaseCharacter->OnCharacterDied.AddDynamic(this, &APTWMiniGameMode::HandlePlayerDeath);
	}
	
	if (APTWPlayerController* PC = Cast<APTWPlayerController>(NewPlayer))
	{
		PC->Client_RefreshTeamOutline(false, MiniGameRule.TeamRule.bUseTeam, bFriendlyOnlyOutline);
	}
	
	APTWPlayerCharacter* TargetCharacter = Cast<APTWPlayerCharacter>(NewPlayer->GetPawn());
	if (!TargetCharacter) return;

	UPTWWeaponComponent* WeaponComp = TargetCharacter->GetWeaponComponent();
	UPTWInventoryComponent* InvenComp = TargetCharacter->GetInventoryComponent();
	if (!WeaponComp || !InvenComp) return;
	
	UPTWItemSpawnManager* SpawnManager = GetWorld()->GetSubsystem<UPTWItemSpawnManager>();
	if (!SpawnManager) return;
	
	SpawnManager->CopyRestartPlayerItems(TargetCharacter);
	
	if (PTWGameState->GetCurrentGamePhase() == EPTWGamePhase::MiniGame)
	{
		RefreshTeamOutlineForAllPlayers(true);
	}
}

//FIXME : 03/11 박태웅 내부함수 코드 삭제
void APTWMiniGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
	UE_LOG(Log_MiniGameMode, Warning, TEXT("[미니게임모드] HandleSeamlessTravelPlayer: %s"), *C->GetName());
	
	Super::HandleSeamlessTravelPlayer(C);

	PlayerReadyToPlay(Cast<APlayerController>(C));
}

void APTWMiniGameMode::PlayerReadyToPlay(APlayerController* Controller)
{
	Super::PlayerReadyToPlay(Controller);

	UE_LOG(Log_MiniGameMode, Warning, TEXT("[MiniGame] PlayerReadyToPlay: %s, %d/%d"), *Controller->GetName(),ReadyPlayer, AllPlayer);
	
	if (!IsValid(PTWGameState) || !Controller) return;

	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (!PlayerController) return;
	
	APTWPlayerState* PTWPlayerState = Controller->GetPlayerState<APTWPlayerState>();
	if (!PTWPlayerState) return;
	
	if (ReadyPlayer >= AllPlayer)
	{
		if (bAllPlayerReady) return;
		bAllPlayerReady = true;

		UE_LOG(Log_MiniGameMode, Log, TEXT("All Player Success Travel"));
		
		FTimerHandle LoadingDelayTimer;
		GetWorldTimerManager().SetTimer(LoadingDelayTimer, FTimerDelegate::CreateLambda([this]()
		{
			if (!bIsGameStarted)
			{
				StartGame();
			}
		}), 3.f, false);
	}
}

#pragma endregion override

#pragma region GameFlow

void APTWMiniGameMode::StartGame()
{
	if (!PTWGameState) return;
	if (bIsGameStarted) return;

	bIsGameStarted = true;
	
	AssignTeam();

	AGameStateBase* GS = GetWorld()->GetGameState();
	if (!GS) return;

	for (APlayerState* PS : GS->PlayerArray)
	{
		if (!PS) continue;

		APlayerController* PC = Cast<APlayerController>(PS->GetOwner());
		if (!PC) continue;

		SpawnDefaultWeapon(PC);
	}
	
	PTWGameState->SetCurrentPhase(EPTWGamePhase::MiniGame);
	
	if (bApplyPropOnStartGame)
	{
		ApplyRoundPropRandom();
	}
	
	//SetInputBlock(false);
	
	WaitingToStartRound();
}

void APTWMiniGameMode::StartChaosEvent()
{
	ChaosEventManager->StartChaosEvent();
}

void APTWMiniGameMode::WaitingToStartRound()
{
	if (!PTWGameState) return;

	// MiniGame 레벨 진입 → 카운트다운 시작
	PTWGameState->AdvanceMiniGameRound();
	
	// 카운트 다운 사용 안하면 바로 라운드 시작
	if (!MiniGameRule.TimeRule.bUseCountDown)
	{
		StartRound();

		return;
	}
	
	if (!PTWGameState->OnCountDownFinished.IsAlreadyBound(this, &APTWMiniGameMode::OnCountDownFinished))
	{
		PTWGameState->OnCountDownFinished.AddDynamic(this, &APTWMiniGameMode::OnCountDownFinished);
	}
	
	StartCountDown();
	PrepareAllPlayersLoadingScreen(ELoadingScreenType::Lobby, NAME_None);
}


void APTWMiniGameMode::StartCountDown()
{
	if (!PTWGameState) return;
	
	RefreshTeamOutlineForAllPlayers(false);
	
	PTWGameState->SetMiniGameCountdown(MiniGameRule.TimeRule.CountDown);
	PTWGameState->SetbMiniGameCountdown(true);

	// 카운트 다운 동안 모든 플레이어 무적 
	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		APTWPlayerState* PTWPlayerState = Cast<APTWPlayerState>(PlayerState);
		if (PTWPlayerState)
		{
			PTWPlayerState->ApplyInvincible(MiniGameRule.TimeRule.CountDown);
		}
	}
	
	GetWorldTimerManager().ClearTimer(CountDownTimerHandle);
	GetWorldTimerManager().SetTimer(CountDownTimerHandle, this, &APTWMiniGameMode::TickCountDown, 1.0f, true, 1.f);
}

void APTWMiniGameMode::TickCountDown()
{
	if (!PTWGameState) return;

	PTWGameState->DecreaseCoundDown();
}

void APTWMiniGameMode::OnCountDownFinished()
{
	UE_LOG(LogTemp, Warning, TEXT("Round Start"));

	if (!PTWGameState) return;
	
	GetWorldTimerManager().ClearTimer(CountDownTimerHandle);
	
	PTWGameState->SetbMiniGameCountdown(false);
	
	StartRound();
}

void APTWMiniGameMode::StartRound()
{
	if (!bApplyPropOnStartGame && bApplyPropOnStartRound)
	{
		ApplyRoundPropRandom();
	}
	
	if (MiniGameRule.TimeRule.bUseTimer)
	{
		StartTimer(MiniGameRule.TimeRule.Timer);
	}

	StartChaosEvent();
	
	if (GetWorld())
	{
		GetWorldTimerManager().SetTimer(CoinSpawnTimerHandle, this, &APTWMiniGameMode::OnCoinSpawnTimerElapsed, CoinSpawnInterval, true);
	}
	
	RefreshTeamOutlineForAllPlayers(true);
}

void APTWMiniGameMode::UpdateTimer()
{
	Super::UpdateTimer();

	StartChaosEvent();
}

void APTWMiniGameMode::EndTimer()
{
	if (bIsGameEnded) return;
	
	Super::EndTimer();
	
	EndRound();
	
}

void APTWMiniGameMode::EndRound()
{
	if (bIsGameEnded) return;
	
	GetWorldTimerManager().ClearTimer(CoinSpawnTimerHandle);
	
	ChaosEventManager->EndChaosEvent();
	
	RefreshTeamOutlineForAllPlayers(false);
	
	if (!PTWGameState) return;
	
	if (PTWGameState->GetCurrentMiniGameRound() >= PTWGameState->GetMaxMiniGameRound())
	{
		EndGame();
		
		return;
	}
	
	WaitingToStartRound();
}

void APTWMiniGameMode::EndGame()
{
	if (bIsGameEnded)
	{
		return;
	}

	bIsGameEnded = true;

	if (!PTWGameState) return;

	PTWGameState->ApplyMiniGameRankScore(MiniGameRule);
	PTWGameState->Multicast_BroadcastMiniGameEnded();

	GetWorldTimerManager().ClearTimer(CountDownTimerHandle);
	GetWorldTimerManager().ClearTimer(CoinSpawnTimerHandle);

	ChaosEventManager->EndChaosEvent();
	
	FTimerHandle ResultSequenceTimerHandle;
	GetWorldTimerManager().SetTimer(ResultSequenceTimerHandle, this, &ThisClass::StartResultSequence, 3.0f, false);
}

void APTWMiniGameMode::TravelLevel()
{
	ResetPlayerRoundData();
	ResetPlayerInventoryID();
	
	if (!PTWGameState) return;
	PTWGameState->ResetChaosItemEntries();
	
	Super::TravelLevel();
}


#pragma endregion GameFlow

#pragma region other

void APTWMiniGameMode::AttachControllerComponent(AController* Controller, UActorComponent* Component)
{
	if (!Controller) return;

	APTWPlayerController* PlayerController = Cast<APTWPlayerController>(Controller);
	if (!PlayerController) return;
	
	if (UActorComponent* BeforeComponent = PlayerController->GetControllerComponent())
	{
		BeforeComponent->UnregisterComponent();
		BeforeComponent->DestroyComponent();
	}
	
	UActorComponent* ActorComponent = Component;

	if (!ActorComponent && ControllerComponentClass)
	{
		ActorComponent = NewObject<UActorComponent>(PlayerController, ControllerComponentClass, NAME_None);
	}
	if (!ActorComponent) return;

	ActorComponent->SetIsReplicated(true);
	PlayerController->AddInstanceComponent(ActorComponent);
	
	ActorComponent->RegisterComponent();

	PlayerController->SetControllerComponent(ActorComponent);

}

void APTWMiniGameMode::SpawnDefaultWeapon(AController* NewPlayer)
{
	
	if (MiniGameRule.LoadoutRule.DefaultWeapon.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[MiniGameMode] SpawnDefaultWeapon Failed: ItemDefinition is NULL. Please set Default Weapon in Blueprint."));
		return;
	}
	
	if (UPTWItemSpawnManager* ItemSpawnManager = GetWorld()->GetSubsystem<UPTWItemSpawnManager>())
	{
		if (APTWPlayerCharacter* PlayerCharacter = Cast<APTWPlayerCharacter>(NewPlayer->GetPawn()))
		{
			for (UPTWItemDefinition* DefaultWeapon: MiniGameRule.LoadoutRule.DefaultWeapon)
			{
				FGameplayTag WeaponTag = DefaultWeapon->WeaponTag;
				ItemSpawnManager->SpawnWeaponActor(PlayerCharacter, DefaultWeapon, WeaponTag);
			}
		}
	}
}

void APTWMiniGameMode::ApplyRoundPropRandom()
{
	if (!HasAuthority()) return;
	if (!PTWGameState) return;
	if (!RoundPropData) return;

	PTWGameState->Server_SetPropData(RoundPropData);
	PTWGameState->Server_SetPropSeed(FMath::Rand());
}

void APTWMiniGameMode::AddKillDeathCount(APlayerState* DeadPlayerState, APlayerState* KillPlayerState)
{
	if (!IsValid(DeadPlayerState)) return;
	
	if (IPTWPlayerDataInterface* DeadPlayerData = Cast<IPTWPlayerDataInterface>(DeadPlayerState))
	{
		DeadPlayerData->AddDeathCount(1);
		DeadPlayerData->SetDeathOrder(CurrentDeathOrder++);
	}

	if (DeadPlayerState == KillPlayerState) return;

	if (!IsValid(KillPlayerState)) return;
	
	if (IPTWPlayerDataInterface* KillPlayerData = Cast<IPTWPlayerDataInterface>(KillPlayerState))
	{
		KillPlayerData->AddKillCount();
		AddRoundScore(KillPlayerState, MiniGameRule.KillRule.KillScore);
		
	}
}

void APTWMiniGameMode::AddRoundScore(APlayerState* ScoreTarget, int32 ScoreValue)
{
	if (!IsValid(ScoreTarget) && ScoreValue == 0) return;
	// 미니 게임 진행 중이 아니면 점수 부여 X
	if (!PTWGameState && PTWGameState->GetCurrentGamePhase() != EPTWGamePhase::MiniGame) return;
	
	if (IPTWPlayerDataInterface* RoundDataInterface = Cast<IPTWPlayerDataInterface>(ScoreTarget))
	{
		if (MiniGameRule.TeamRule.bUseTeam && MiniGameRule.TeamRule.bShareScoreWithinTeam)
		{
			if (!PTWGameState) return;

			PTWGameState->AddTeamScore(ScoreTarget, ScoreValue);
			RoundDataInterface->AddScore(ScoreValue);
		}
		else
		{
			RoundDataInterface->AddScore(ScoreValue);
		}
	}
	PTWGameState->UpdateRanking(MiniGameRule);
	CheckEndGameCondition();
}

void APTWMiniGameMode::RespawnPlayer(APTWPlayerController* SpawnPlayerController)
{
	if (!SpawnPlayerController) return;
	if (!SpawnComponent) return;

	SpawnComponent->RespawnPlayer(SpawnPlayerController);
	
}

void APTWMiniGameMode::HandleRespawn(APTWPlayerController* PlayerController)
{
	if (!SpawnComponent) return;

	SpawnComponent->HandleRespawn(PlayerController);
}

void APTWMiniGameMode::CheckEndGameCondition()
{
	if (!WinConditionComponent) return;

	WinConditionComponent->CheckEndGameCondition();
}

void APTWMiniGameMode::AssignTeam()
{
	if (!MiniGameRule.TeamRule.bUseTeam) return;
	if (!PTWGameState) return;

	PTWGameState->GetTeams().Empty();

	int32 NumTeams = MiniGameRule.TeamRule.NumTeams;
	for (int i = 0; i < NumTeams; i++)
	{
		FPTWTeamInfo NewTeam;
		NewTeam.TeamID = i;

		PTWGameState->GetTeams().Add(NewTeam);
	}

	// 플레이어 균등하게 배분
	TArray<APlayerState*> Players = PTWGameState->PlayerArray;
	for (int i = 0; i < Players.Num(); i++)
	{
		int32 AssignTeamId = i % NumTeams;
		PTWGameState->GetTeams()[AssignTeamId].Members.Add(Players[i]);
		Cast<IPTWPlayerDataInterface>(Players[i])->SetTeamId(AssignTeamId); 
	}
}

void APTWMiniGameMode::OnCoinSpawnTimerElapsed()
{
	if (UPTWItemSpawnManager* SpawnManager = GetWorld()->GetSubsystem<UPTWItemSpawnManager>())
	{
		SpawnManager->SpawnCoinInRandomVolume();
	}
}

void APTWMiniGameMode::StartResultSequence()
{
	if (!ResultSequenceComponent) return;
	ResultSequenceComponent->StartResultSequence();
}

bool APTWMiniGameMode::IsWinner(APTWPlayerState* InPlayerState)
{
	if (!InPlayerState) return false;
	
	if (MiniGameRule.WinConditionRule.WinType == EPTWWinType::Survival)
	{
		return (InPlayerState->GetDeathOrder() == 0);
	}
	
	return InPlayerState == PTWGameState->GetRankedPlayers()[0];
}

void APTWMiniGameMode::ExitSpectatorMode(AController* Controller)
{
	APlayerController* PC = Cast<APlayerController>(Controller);
	if (!PC) return;

	if (PC->GetStateName() == NAME_Spectating)
	{
		PC->ChangeState(NAME_Playing);
	}

	if (PC->PlayerState)
	{
		PC->PlayerState->SetIsSpectator(false);
		PC->PlayerState->SetIsOnlyASpectator(false);
	}

	PC->SetViewTarget(PC);
}

void APTWMiniGameMode::SpawnPlayerSavedItems(AController* Controller)
{
	if (!Controller) return;

	APTWPlayerState* PS = Controller->GetPlayerState<APTWPlayerState>();
	APawn* Pawn = Controller->GetPawn();

	if (PS && Pawn)
	{
		if (UPTWItemSpawnManager* SpawnSys = GetWorld()->GetSubsystem<UPTWItemSpawnManager>())
		{
			SpawnSys->SpawnAndGiveItems(PS);
			UE_LOG(LogTemp, Log, TEXT("[GameMode] SpawnPlayerSavedItems Success for %s"), *PS->GetPlayerName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] Failed to Spawn Items. PS or Pawn is invalid."));
	}
}

bool APTWMiniGameMode::ShouldUseTeamOutline() const
{
	return MiniGameRule.TeamRule.bUseTeam;
}

void APTWMiniGameMode::RefreshTeamOutlineForAllPlayers(bool bEnable)
{
	UWorld* World = GetWorld();
	if (!World) return;

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APTWPlayerController* PC = Cast<APTWPlayerController>(It->Get());
		if (!PC) continue;

		PC->Client_RefreshTeamOutline(bEnable, MiniGameRule.TeamRule.bUseTeam, bFriendlyOnlyOutline);
	}
}

void APTWMiniGameMode::PlayerMontageStop(APTWPlayerCharacter* TargetCharacter)
{
	if (TargetCharacter->GetMesh1P())
	{
		UAnimInstance* AnimInst = TargetCharacter->GetMesh1P()->GetAnimInstance();
		if (AnimInst)
		{
			AnimInst->Montage_Stop(0.2f); 
		}
	}
	if (TargetCharacter->GetMesh3P())
	{
		UAnimInstance* AnimInst = TargetCharacter->GetMesh3P()->GetAnimInstance();
		if (AnimInst)
		{
			AnimInst->Montage_Stop(0.2f); 
		}
	}
}

void APTWMiniGameMode::DeathPlayerWeaponHandler(UPTWInventoryComponent* InvenComp)
{
	const TArray<TObjectPtr<UPTWWeaponInstance>>& WeaponArr = InvenComp->GetWeaponArray();
		
	for (const auto& WeaponInst : WeaponArr)
	{
		if (WeaponInst)
		{
			WeaponInst->SpawnedWeapon1P->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			WeaponInst->SpawnedWeapon3P->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			WeaponInst->DestroySpawnedActors();
		}
	}
	InvenComp->SendEquipEventToASC(InvenComp->GetCurrentSlotIndex());
}

#pragma endregion other

#pragma endregion protected

#pragma region private

void APTWMiniGameMode::ApplyMiniGameTag(AController* NewPlayer)
{
	if (!MiniGameEffectClass) return;
	
	if (APTWBaseCharacter* PTWBaseCharacter = Cast<APTWBaseCharacter>(NewPlayer->GetPawn()))
	{
		UAbilitySystemComponent* AbilitySystemComponent = PTWBaseCharacter->GetAbilitySystemComponent();
		if (!AbilitySystemComponent) return;
	
		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
		
		PTWBaseCharacter->ApplyGameplayEffectToSelf(MiniGameEffectClass, 1, Context);
	}
}

void APTWMiniGameMode::ResetPlayerRoundData()
{
	if (!PTWGameState) return;
	for (APlayerState* PlayerState : PTWGameState->PlayerArray)
	{
		if (IPTWPlayerDataInterface* RoundDataInterface = Cast<IPTWPlayerDataInterface>(PlayerState))
		{
			RoundDataInterface->ResetRoundData();
		}
	}
}

void APTWMiniGameMode::ResetPlayerInventoryID()
{
	if (!PTWGameState) return;
	
	for (APlayerState* PlayerState : PTWGameState->PlayerArray)
	{
		APTWPlayerState* PTWPlayerState = Cast<APTWPlayerState>(PlayerState);
		if (!PTWPlayerState) continue;

		PTWPlayerState->ResetInventoryItemId();
	}
}

void APTWMiniGameMode::InitPlayerHealth(AController* Controller)
{
	APTWPlayerController* PlayerController = Cast<APTWPlayerController>(Controller);
	if (!PlayerController) return;

	APTWPlayerState* PlayerState = PlayerController->GetPlayerState<APTWPlayerState>();
	if (!PlayerState) return;

	UPTWAttributeSet* AttributeSet = Cast<UPTWAttributeSet>(PlayerState->GetAttributeSet());
	if (!AttributeSet) return;

	AttributeSet->SetHealth(AttributeSet->GetMaxHealth());
}

void APTWMiniGameMode::RemoveTags(AController* Controller)
{
	if (!Controller) return;

	APTWPlayerState* PlayerState = Controller->GetPlayerState<APTWPlayerState>();
	if (!PlayerState) return;

	UAbilitySystemComponent* ASC = PlayerState->GetAbilitySystemComponent();
	if (!ASC) return;
	
	FGameplayTagContainer GameplayTagContainer;
	ASC->GetOwnedGameplayTags(GameplayTagContainer);

	for (const FGameplayTag& Tag : GameplayTagContainer)
	{
		ASC->RemoveLooseGameplayTag(Tag);
	}
}

void APTWMiniGameMode::SetOldPlayerItemInstanceOuter(TArray<TObjectPtr<UPTWItemInstance>> ItemArr)
{
	for (auto Item : ItemArr)
	{
		Item->Rename(nullptr, this);
	}
}

#pragma endregion private

