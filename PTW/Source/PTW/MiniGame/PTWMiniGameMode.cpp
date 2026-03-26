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

APTWMiniGameMode::APTWMiniGameMode()
{
	ChaosEventManager = CreateDefaultSubobject<UPTWChaosEventManager>(TEXT("ChaosEventManager"));

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
	
	// 카오스 이벤트 태그 적용 테스트
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
	/*
	if (IsValid(PTWGameState) && PTWGameState->IsMatchInProgress())
	{
		if(PTWGameState->GetCurrentGamePhase() == EPTWGamePhase::MiniGame)
		{
			NewPlayer->StartSpectatingOnly();
			return;
		}
	}*/

	//FIXME : 03/09 박태웅 테스트로 주석처리
	//FIXME : 임시로 난입플레이어도 관전상태해제
	//ExitSpectatorMode(NewPlayer);
	
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	
	if (!IsValid(PTWGameState)) return;
	if (!NewPlayer) return;
	
	APTWPlayerState* PlayerState = NewPlayer->GetPlayerState<APTWPlayerState>();
	if (!IsValid(PlayerState)) return;
	
	//SetInputBlock(true);
	
	PTWGameState->AddRankedPlayer(PlayerState);

	//FIXME : 03/09 박태웅 테스트로 주석처리
	//FIXME : 임시로 난입플레이어도 리스타트 플레이어 시키기
	//if (NewPlayer->GetPawn() == nullptr)
	//{
	//	RestartPlayer(NewPlayer);
	//}

	AttachControllerComponent(NewPlayer);
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
	// CurrentCountDown = StartCountDownTime;
	//
	// UE_LOG(LogTemp, Warning, TEXT("StartCountDown : %d"), CurrentCountDown);
	//
	// if (APTWGameState* GS = GetGameState<APTWGameState>())
	// {
	// 	GS->SetMiniGameCountdown(CurrentCountDown);
	// }

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

void APTWMiniGameMode::ApplyRoundPropRandom()
{
	if (!HasAuthority()) return;
	if (!PTWGameState) return;
	if (!RoundPropData) return;

	PTWGameState->Server_SetPropData(RoundPropData);
	PTWGameState->Server_SetPropSeed(FMath::Rand());
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

void APTWMiniGameMode::EndTimer()
{
	if (bIsGameEnded) return;
	
	Super::EndTimer();
	
	EndRound();
	//UE_LOG(LogTemp, Warning, TEXT("EndTimer PTWMiniGameMode"));
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

void APTWMiniGameMode::CheckEndGameCondition()
{
	switch (MiniGameRule.WinConditionRule.WinType)
	{
	case EPTWWinType::Survival:
		CheckSurvivalCondition();
		break;
	case EPTWWinType::Target:
		CheckTargetCondition();
		break;
	default:
		break;
	}
}

void APTWMiniGameMode::CheckSurvivalCondition()
{
	if (!PTWGameState) return;
	
	// 팀전일 경우 생존 플레이어의 팀을 확인하고 한팀만 있으면 승리
	if (MiniGameRule.TeamRule.bUseTeam)
	{
		TSet<int32> AliveTeams;
		for (APlayerState* Player : PTWGameState->AlivePlayers)
		{
			if (IPTWPlayerRoundDataInterface* PlayerRoundDataInterface = Cast<IPTWPlayerRoundDataInterface>(Player))
			{
				AliveTeams.Add(PlayerRoundDataInterface->GetTeamId());
			}
		}
		
		if (AliveTeams.Num() == 1)
		{
			PTWGameState->SetWinTeamId(*AliveTeams.begin());
			EndGame();
		}
		else if (AliveTeams.IsEmpty())
		{
			// 생존 팀이 없을 경우 가장 마지막에 죽은 플레이어의 팀 승리
			if (IPTWPlayerRoundDataInterface* Last = FindLastDeadPlayer())
			{
				PTWGameState->SetWinTeamId(Last->GetTeamId());
				EndGame();
			}
		}
	}
	else
	{
		if (PTWGameState->AlivePlayers.Num() == 1)
		{
			EndGame();
		}
		else if (PTWGameState->AlivePlayers.IsEmpty())
		{
			if (IPTWPlayerRoundDataInterface* Last = FindLastDeadPlayer())
			{
				PTWGameState->AlivePlayers.Add(Cast<APlayerState>(Last));
				EndGame();
			}
		}
	}
}

void APTWMiniGameMode::CheckTargetCondition()
{
	if (!PTWGameState) return;
	// 팀 및 점수 공유 사용하면 팀 점수 기준으로 승리 체크 후 승리 팀 id 전달
	if (MiniGameRule.TeamRule.bUseTeam && MiniGameRule.TeamRule.bShareScoreWithinTeam)
	{
		for (const FPTWTeamInfo& TeamInfo : PTWGameState->GetTeams())
		{
			if (TeamInfo.TeamScore >= MiniGameRule.WinConditionRule.TargetValue)
			{
				PTWGameState->SetWinTeamId(TeamInfo.TeamID);
				EndGame();
				return;
			}
		}
		
	}
	else
	{
		TArray<APTWPlayerState*> RankedPlayers = PTWGameState->GetRankedPlayers();
		if (RankedPlayers.IsEmpty()) return;

		if (RankedPlayers[0]->GetPlayerRoundData().Score >= MiniGameRule.WinConditionRule.TargetValue)
		{
			if (MiniGameRule.TeamRule.bUseTeam && !MiniGameRule.TeamRule.bShareScoreWithinTeam)
			{
				PTWGameState->SetWinTeamId(RankedPlayers[0]->GetTeamId());
			}
			EndGame();
		}
	}
}

void APTWMiniGameMode::RestartPlayer(AController* NewPlayer)
{
	if (!NewPlayer) return;
	
	Super::RestartPlayer(NewPlayer);
	
	
	if (!IsValid(PTWGameState)) return;
	
	// if (PlayerStarts.Num() == 0)
	// {
	// 	Super::RestartPlayer(NewPlayer);
	// }
	// else
	// {
	// 	if(PlayerStartCount >= PlayerStarts.Num()-1)
	// 	{
	// 		PlayerStartCount = 0;
	// 	}
	// 	RestartPlayerAtPlayerStart(NewPlayer, PlayerStarts[PlayerStartCount++]);
	// }

	APlayerState* PlayerState = NewPlayer->GetPlayerState<APlayerState>();
	if (!PlayerState) return;

	//SetInputBlock(NewPlayer, true);
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
	
	
	if (const TArray<FWeaponPair>* WeaponPairsPtr = InvenComp->GetWeaponActorsArr(NewPlayer))
	{
		for (const FWeaponPair& WeaponPair : *WeaponPairsPtr)
		{
			UPTWWeaponInstance* WeaponInst = WeaponPair.Weapon3P->GetWeaponItemInstance();
			if (WeaponInst && WeaponInst->ItemDef)
			{
				WeaponComp->SpawnedWeapons.Remove(WeaponInst->ItemDef->WeaponTag);
				WeaponComp->AttachWeaponToSocket(
					WeaponPair.Weapon1P,
					WeaponPair.Weapon3P,
					WeaponInst->ItemDef->WeaponTag
				);
			}
		}
	}
	
	UPTWItemSpawnManager* SpawnManager = GetWorld()->GetSubsystem<UPTWItemSpawnManager>();
	if (!SpawnManager) return;
	
	SpawnManager->CopyRestartPlayerItems(TargetCharacter);
}


void APTWMiniGameMode::SpawnDefaultWeapon(AController* NewPlayer)
{
	// if (!ItemDefinition)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("[MiniGameMode] SpawnDefaultWeapon Failed: ItemDefinition is NULL. Please set Default Weapon in Blueprint."));
	// 	return;
	// }
	
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

			// FGameplayTag WeaponTag = ItemDefinition->WeaponTag;
			// ItemSpawnManager->SpawnWeaponActor(PlayerCharacter, ItemDefinition, WeaponTag);
		}
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
		
		
		
		//
		// FSavedWeaponData SavedData;
		// for (const auto& WeaponInst : WeaponArr)
		// {
		// 	FWeaponPair WeaponPair;
		// 	WeaponPair.Weapon1P = WeaponInst->SpawnedWeapon1P;
		// 	WeaponPair.Weapon3P = WeaponInst->SpawnedWeapon3P;
		// 	SavedData.WeaponArray.Add(WeaponPair);
		// }
		//
		// InvenComp->SetSavedWeaponActor(DeadPawn->GetController(), SavedData);
		InvenComp->SendEquipEventToASC(InvenComp->GetCurrentSlotIndex());
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

void APTWMiniGameMode::AddKillDeathCount(APlayerState* DeadPlayerState, APlayerState* KillPlayerState)
{
	if (!IsValid(DeadPlayerState)) return;
	
	if (IPTWPlayerRoundDataInterface* DeadPlayerData = Cast<IPTWPlayerRoundDataInterface>(DeadPlayerState))
	{
		DeadPlayerData->AddDeathCount(1);
		DeadPlayerData->SetDeathOrder(CurrentDeathOrder++);
	}

	if (DeadPlayerState == KillPlayerState) return;

	if (!IsValid(KillPlayerState)) return;
	
	if (IPTWPlayerRoundDataInterface* KillPlayerData = Cast<IPTWPlayerRoundDataInterface>(KillPlayerState))
	{
		KillPlayerData->AddKillCount();
		AddRoundScore(KillPlayerState, MiniGameRule.KillRule.KillScore);
		
	}
	//CheckEndGameCondition();
}

void APTWMiniGameMode::AddRoundScore(APlayerState* ScoreTarget, int32 ScoreValue)
{
	if (!IsValid(ScoreTarget) && ScoreValue == 0) return;
	// 미니 게임 진행 중이 아니면 점수 부여 X
	if (!PTWGameState && PTWGameState->GetCurrentGamePhase() != EPTWGamePhase::MiniGame) return;
	
	if (IPTWPlayerRoundDataInterface* RoundDataInterface = Cast<IPTWPlayerRoundDataInterface>(ScoreTarget))
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
		Cast<IPTWPlayerRoundDataInterface>(Players[i])->SetTeamId(AssignTeamId); 
	}
}

IPTWPlayerRoundDataInterface* APTWMiniGameMode::FindLastDeadPlayer()
{
	IPTWPlayerRoundDataInterface* LastInterface = nullptr;
	int32 HighDeathOrder = -1;

	for (APlayerState* Player : PTWGameState->PlayerArray)
	{
		if (IPTWPlayerRoundDataInterface* Interface = Cast<IPTWPlayerRoundDataInterface>(Player))
		{
			if (Interface->GetDeathOrder() > HighDeathOrder)
			{
				HighDeathOrder = Interface->GetDeathOrder();
				LastInterface = Interface;
			}
		}
	}
	return LastInterface;
}

void APTWMiniGameMode::ResetPlayerRoundData()
{
	if (!PTWGameState) return;
	for (APlayerState* PlayerState : PTWGameState->PlayerArray)
	{
		if (IPTWPlayerRoundDataInterface* RoundDataInterface = Cast<IPTWPlayerRoundDataInterface>(PlayerState))
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

void APTWMiniGameMode::RespawnPlayer(APTWPlayerController* SpawnPlayerController)
{
	if (MiniGameRule.SpawnRule.bUseRespawn == false) return;
	
	if (IsValid(SpawnPlayerController))
	{
		GetWorldTimerManager().ClearTimer(SpawnPlayerController->RespawnTimerHandle);		// 방어 코드
		TWeakObjectPtr<ThisClass> WeakThis = this;
		TWeakObjectPtr<APTWPlayerController> WeakDeadController = SpawnPlayerController;
		GetWorldTimerManager().SetTimer(WeakDeadController->RespawnTimerHandle, [WeakThis, WeakDeadController, this]()
		{
			WeakThis->HandleRespawn(WeakDeadController.Get());
			
		}, MiniGameRule.SpawnRule.RespawnDelay, false);
	}
}

void APTWMiniGameMode::HandleRespawn(APTWPlayerController* PlayerController)
{
	if (!IsValid(PlayerController)) return;

	RestartPlayer(PlayerController);

	if (MiniGameRule.SpawnRule.bUseSpawnProtection)
	{
		APTWPlayerState* PlayerState = PlayerController->GetPlayerState<APTWPlayerState>();
		if (!PlayerState) return;

		PlayerState->ApplyInvincible(MiniGameRule.SpawnRule.SpawnProtectionTime);
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

void APTWMiniGameMode::SetOldPlayerItemInstanceOuter(TArray<TObjectPtr<UPTWItemInstance>> ItemArr)
{
	for (auto Item : ItemArr)
	{
		Item->Rename(nullptr, this);
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
	UWorld* World = GetWorld();
	if (!World) return;
	
	AActor* ResultCamera = nullptr;
	TArray<AActor*> FoundActors;

	UGameplayStatics::GetAllActorsWithTag(World, FName("ResultCamera"), FoundActors);
	if (FoundActors.Num() > 0) ResultCamera = FoundActors[0];

	AActor* WinSpot = nullptr;
	UGameplayStatics::GetAllActorsWithTag(World, FName("WinSpot"), FoundActors);
	if (FoundActors.Num() > 0) WinSpot = FoundActors[0];

	AActor* LoseSpot = nullptr;
	UGameplayStatics::GetAllActorsWithTag(World, FName("LoseSpot"), FoundActors);
	if (FoundActors.Num() > 0) LoseSpot = FoundActors[0];
	
	if (!PTWGameState) return;
	PTWGameState->SetCurrentPhase(EPTWGamePhase::MiniGameResult);
	
	int32 WinnerCount = 0;
	int32 LoserCount = 0;
	
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APTWPlayerController* PC = Cast<APTWPlayerController>(It->Get());
		if (!PC) continue;

		APTWPlayerState* PS = PC->GetPlayerState<APTWPlayerState>();
		if (!PS) continue;

		PC->SetIgnoreMoveInput(true);
		PC->SetIgnoreLookInput(true);

		if (APawn* OriginalPawn = PC->GetPawn())
		{
			OriginalPawn->SetActorHiddenInGame(true);
			OriginalPawn->SetActorEnableCollision(false);
		}

		if (ResultCamera)
		{
			FViewTargetTransitionParams Params;
			Params.BlendTime = 1.0f;
			Params.BlendFunction = EViewTargetBlendFunction::VTBlend_Linear;
			Params.bLockOutgoing = true;

			PC->ClientSetViewTarget(ResultCamera, Params);
		}

		if (ResultCharacterClass)
		{
			bool bIsWinner = (PS->GetDeathOrder() == 0);
			FVector SpawnLoc = FVector::ZeroVector;
			FRotator SpawnRot = FRotator::ZeroRotator;

			if (bIsWinner && WinSpot)
			{
				SpawnLoc = WinSpot->GetActorLocation() + (WinSpot->GetActorRightVector() * 100.0f * WinnerCount++);
				SpawnRot = WinSpot->GetActorRotation();
			}
			else if (!bIsWinner && LoseSpot)
			{
				SpawnLoc = LoseSpot->GetActorLocation() + (LoseSpot->GetActorRightVector() * 100.0f * LoserCount++);
				SpawnRot = LoseSpot->GetActorRotation();
			}

			if (!SpawnLoc.IsZero())
			{
				FActorSpawnParameters Params;
				Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				APTWResultCharacter* ResultChar = World->SpawnActor<APTWResultCharacter>(ResultCharacterClass, SpawnLoc, SpawnRot, Params);

				if (ResultChar)
				{
					ResultChar->InitializeResult(bIsWinner);
				}
			}
		}
	}
	GetWorldTimerManager().SetTimer(ResultTimerHandle, this, &APTWMiniGameMode::FinishEndGameSequence, ResultSequenceDuration, false);

}

void APTWMiniGameMode::FinishEndGameSequence()
{
	ResetPlayerRoundData();
	ResetPlayerInventoryID();
	
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APTWPlayerController* PC = Cast<APTWPlayerController> (It->Get());
		if (!PC) continue;

		PC->ChangeState(NAME_Playing);
		PC->ClientGotoState(NAME_Playing);

		if (PC->PlayerState)
		{
			PC->PlayerState->SetIsSpectator(false);
			PC->PlayerState->SetIsOnlyASpectator(false);
		}

		PC->SetControllerComponent(nullptr);
	}

	// 로비로 이동 전 구매한 카오스 아이템 목록 삭제
	if (!PTWGameState) return;
	PTWGameState->ResetChaosItemEntries();
	
	TravelLevel();
}

// FIXME : 임시로 관전상태 해제테스트
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

void APTWMiniGameMode::StartChaosEvent()
{
	ChaosEventManager->StartChaosEvent();
}
