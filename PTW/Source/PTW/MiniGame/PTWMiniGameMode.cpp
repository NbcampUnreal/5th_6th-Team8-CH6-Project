// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWMiniGameMode.h"

#include "PTW/System/PTWItemSpawnManager.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "GAS/PTWAttributeSet.h"
#include "EngineUtils.h"
#include "Manager/PTWChaosEventManager.h"
#include "PTW/Inventory/PTWItemDefinition.h"

class UPTWScoreSubsystem;

APTWMiniGameMode::APTWMiniGameMode()
{
	ChaosEventManager = CreateDefaultSubobject<UPTWChaosEventManager>(TEXT("ChaosEventManager"));
}

void APTWMiniGameMode::AddWinPoint(AActor* Actor, int32 AddPoint)
{
	
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

void APTWMiniGameMode::BeginPlay()
{
	Super::BeginPlay();

	//for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	//{
	//	PlayerStarts.Add(*It);
	//}
	//StartGame();
	
	// 카오스 이벤트 태그 적용 테스트
	if (!PTWGameState) return;
	ChaosEventManager->InitGameState(PTWGameState);
	ChaosEventManager->ApplyChaosEvent();
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

	if (PTWGameState->PlayerArray.Num() >= AllPlayer)
	{
		if (bAllPlayerReady) return;
		bAllPlayerReady = true;
		
		FTimerHandle LoadingDelayTimer;
		GetWorldTimerManager().SetTimer(LoadingDelayTimer, this, &APTWMiniGameMode::StartGame, 3.f);
	}
	
}

void APTWMiniGameMode::StartGame()
{
	if (!PTWGameState) return;
	
	PTWGameState->SetCurrentPhase(EPTWGamePhase::MiniGame);
	
	SetInputBlock(false);
	
	WaitingToStartRound();
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
	
	PTWGameState->SetMiniGameCountdown(MiniGameRule.TimeRule.CountDown);
	PTWGameState->SetbMiniGameCountdown(true);
	
	GetWorldTimerManager().ClearTimer(CountDownTimerHandle);
	GetWorldTimerManager().SetTimer(CountDownTimerHandle, this, &APTWMiniGameMode::TickCountDown, 1.0f, true, 1.f);
}

void APTWMiniGameMode::TickCountDown()
{
	// CurrentCountDown--;
	//
	// if (APTWGameState* GS = GetGameState<APTWGameState>())
	// {
	// 	GS->SetMiniGameCountdown(CurrentCountDown);
	// }

	if (!PTWGameState) return;

	PTWGameState->DecreaseCoundDown();
	
	// if (CurrentCountDown > 0)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("CountDown : %d"), CurrentCountDown);
	// 	return;
	// }
	// // 0이되면 카운트 다운 종료 -> 라운드 시작
	// GetWorldTimerManager().ClearTimer(CountDownTimerHandle);
	//
	// OnCountDownFinished();
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
	Super::EndTimer();
	
	EndRound();
	//UE_LOG(LogTemp, Warning, TEXT("EndTimer PTWMiniGameMode"));
}

void APTWMiniGameMode::EndRound()
{
	//ClearTimer();

	GetWorldTimerManager().ClearTimer(CoinSpawnTimerHandle);

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
	if (!PTWGameState) return;
	
	PTWGameState->ApplyMiniGameRankScore(MiniGameRule);
	ResetPlayerRoundData();
	ResetPlayerInventoryID();

	FTimerHandle EndGameDelayTimer;
	GetWorldTimerManager().SetTimer(EndGameDelayTimer, this, &APTWMiniGameMode::TravelLevel, 5.f);
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


void APTWMiniGameMode::StartRound()
{
	if (MiniGameRule.TimeRule.bUserTimer)
	{
		StartTimer(MiniGameRule.TimeRule.Timer);
	}

	if (GetWorld())
	{
		GetWorldTimerManager().SetTimer(CoinSpawnTimerHandle, this, &APTWMiniGameMode::OnCoinSpawnTimerElapsed, CoinSpawnInterval, true);
	}
}

void APTWMiniGameMode::CheckEndGameCondition()
{
	if (!IsValid(PTWGameState)) return;
	
	if (MiniGameRule.WinConditionRule.WinType == EPTWWinType::Survival)
	{
		if (PTWGameState->AlivePlayers.Num() <= 1)
		{
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
	
	ApplyMiniGameTag(NewPlayer);
	InitPlayerHealth(NewPlayer);
	SpawnDefaultWeapon(NewPlayer);
	
	if (APTWBaseCharacter* BaseCharacter = Cast<APTWBaseCharacter>(NewPlayer->GetPawn()))
	{
		BaseCharacter->OnCharacterDied.RemoveDynamic(this, &APTWMiniGameMode::HandlePlayerDeath);
		BaseCharacter->OnCharacterDied.AddDynamic(this, &APTWMiniGameMode::HandlePlayerDeath);
	}
}

void APTWMiniGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
	Super::HandleSeamlessTravelPlayer(C);
	
}

void APTWMiniGameMode::SpawnDefaultWeapon(AController* NewPlayer)
{
	if (!ItemDefinition)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MiniGameMode] SpawnDefaultWeapon Failed: ItemDefinition is NULL. Please set Default Weapon in Blueprint."));
		return;
	}
	
	if (UPTWItemSpawnManager* ItemSpawnManager = GetWorld()->GetSubsystem<UPTWItemSpawnManager>())
	{
		if (APTWPlayerCharacter* PlayerCharacter = Cast<APTWPlayerCharacter>(NewPlayer->GetPawn()))
		{
			FGameplayTag RifleTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Gun.Rifle"));

			ItemSpawnManager->SpawnWeaponActor(PlayerCharacter, ItemDefinition, RifleTag);
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
	
	UpdatePlayerRoundData(DeadPlayerState, KillPlayerState);
	
	if (!PTWGameState) return;
	PTWGameState->UpdateRanking();
	PTWGameState->AlivePlayers.Remove(DeadPlayerState);

	CheckEndGameCondition();
	RespawnPlayer(DeadPlayerController);
}

void APTWMiniGameMode::UpdatePlayerRoundData(APlayerState* DeadPlayerState, APlayerState* KillPlayerState)
{
	if (IsValid(DeadPlayerState))
	{
		if (IPTWPlayerRoundDataInterface* DeadPlayerData = Cast<IPTWPlayerRoundDataInterface>(DeadPlayerState))
		{
			DeadPlayerData->AddDeathCount(1);
		}
	}

	if (IsValid(KillPlayerState))
	{
		if (IPTWPlayerRoundDataInterface* KillPlayerData = Cast<IPTWPlayerRoundDataInterface>(KillPlayerState))
		{
			KillPlayerData->AddKillCount();
			KillPlayerData->AddScore(1);
		}
	}
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
		GetWorldTimerManager().SetTimer(WeakDeadController->RespawnTimerHandle, [WeakThis, WeakDeadController]()
		{
			if (WeakThis.IsValid() && WeakDeadController.IsValid())		// 파괴 방어
			{
				WeakThis->RestartPlayer(WeakDeadController.Get());
			}
		}, MiniGameRule.SpawnRule.RespawnDelay, false);
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

void APTWMiniGameMode::OnCoinSpawnTimerElapsed()
{
	if (UPTWItemSpawnManager* SpawnManager = GetWorld()->GetSubsystem<UPTWItemSpawnManager>())
	{
		SpawnManager->SpawnCoinInRandomVolume();
	}
}



