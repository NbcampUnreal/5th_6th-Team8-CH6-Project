// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWMiniGameMode.h"

#include "PTW/System/PTWItemSpawnManager.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "GameFramework/PlayerStart.h"
#include "GAS/PTWAttributeSet.h"
#include "System/PTWScoreSubsystem.h"
#include "EngineUtils.h"
#include "PTW/Inventory/PTWItemDefinition.h"

class UPTWScoreSubsystem;

APTWMiniGameMode::APTWMiniGameMode()
{
	
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
		PTWGameState->SetCurrentPhase(EPTWGamePhase::MiniGame);
	}
	
}
void APTWMiniGameMode::BeginPlay()
{
	Super::BeginPlay();

	//for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	//{
	//	PlayerStarts.Add(*It);
	//}
	
	if (APTWGameState* GS = GetGameState<APTWGameState>())
	{
		// MiniGame 레벨 진입 → 카운트다운 시작
		GS->SetbMiniGameCountdown(true);
	}

	StartCountDown();
}

void APTWMiniGameMode::EndTimer()
{

	if (!PTWGameState) return;
	
	PTWGameState->ApplyMiniGameRankScore(MiniGameRule);
	ResetPlayerRoundData();
	ResetPlayerInventoryID();

	
	Super::EndTimer();
	//UE_LOG(LogTemp, Warning, TEXT("EndTimer PTWMiniGameMode"));
}

void APTWMiniGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	//SpawnDefaultWeapon(NewPlayer);
}

void APTWMiniGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	if (!IsValid(PTWGameState)) return;

	APTWPlayerState* PlayerState = NewPlayer->GetPlayerState<APTWPlayerState>();
	if (!IsValid(PlayerState)) return;

	PTWGameState->AddRankedPlayer(PlayerState);
}

void APTWMiniGameMode::RestartPlayer(AController* NewPlayer)
{
	if (!NewPlayer) return;

	Super::RestartPlayer(NewPlayer);
	
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

	if (!MiniGameEffectClass) return;
	
	if (APTWBaseCharacter* PTWBaseCharacter = Cast<APTWBaseCharacter>(NewPlayer->GetPawn()))
	{
		UAbilitySystemComponent* AbilitySystemComponent = PTWBaseCharacter->GetAbilitySystemComponent();
		if (!AbilitySystemComponent) return;
	
		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
		
		PTWBaseCharacter->ApplyGameplayEffectToSelf(MiniGameEffectClass, 1, Context);
	}
	
	InitPlayerHealth(NewPlayer);
	SpawnDefaultWeapon(NewPlayer);
	
	if (APTWBaseCharacter* BaseCharacter = Cast<APTWBaseCharacter>(NewPlayer->GetPawn()))
	{
		BaseCharacter->OnCharacterDied.RemoveDynamic(this, &APTWMiniGameMode::HandlePlayerDeath);
		BaseCharacter->OnCharacterDied.AddDynamic(this, &APTWMiniGameMode::HandlePlayerDeath);
	}
}

void APTWMiniGameMode::SpawnDefaultWeapon(AController* NewPlayer)
{
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
		if (!PTWPlayerState) return;

		PTWPlayerState->ResetInventoryItemId();
	}
}

void APTWMiniGameMode::RespawnPlayer(APTWPlayerController* SpawnPlayerController)
{
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
		}, 5.0f, false);
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

void APTWMiniGameMode::StartCountDown()
{
	CurrentCountDown = StartCountDownTime;
	
	UE_LOG(LogTemp, Warning, TEXT("StartCountDown : %d"), CurrentCountDown);
	
	if (APTWGameState* GS = GetGameState<APTWGameState>())
	{
		GS->SetMiniGameCountdown(CurrentCountDown);
	}

	GetWorldTimerManager().ClearTimer(CountDownTimerHandle);
	
	GetWorldTimerManager().SetTimer(CountDownTimerHandle, this, &APTWMiniGameMode::TickCountDown, 1.0f, true);
}

void APTWMiniGameMode::TickCountDown()
{
	CurrentCountDown--;

	if (APTWGameState* GS = GetGameState<APTWGameState>())
	{
		GS->SetMiniGameCountdown(CurrentCountDown);
	}

	if (CurrentCountDown > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("CountDown : %d"), CurrentCountDown);
		return;
	}
	// 0이되면 카운트 다운 종료 -> 라운드 시작
	GetWorldTimerManager().ClearTimer(CountDownTimerHandle);
	
	OnCountDownFinished();
}

void APTWMiniGameMode::OnCountDownFinished()
{
	UE_LOG(LogTemp, Warning, TEXT("Round Start"));

	if (APTWGameState* GS = GetGameState<APTWGameState>())
	{
		GS->SetbMiniGameCountdown(false);
	}

	StartTimer(RoundPlayTime);
}




