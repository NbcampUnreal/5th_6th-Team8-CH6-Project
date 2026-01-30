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

	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		PlayerStarts.Add(*It);
	}
	
	StartTimer(MiniGameTime);
}

void APTWMiniGameMode::EndTimer()
{
	ApplyMiniGameRankScore();
	ResetPlayerRoundData();
	
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

	if (!PTWGameState) return;

	if (APTWPlayerState* PlayerState = NewPlayer->GetPlayerState<APTWPlayerState>())
	{
		PTWGameState->AddRankedPlayer(PlayerState);
	}
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
	// 	int32 RandomInt = FMath::RandRange(0, PlayerStarts.Num()-1);
	// 	RestartPlayerAtPlayerStart(NewPlayer, PlayerStarts[RandomInt]);
	// }
	
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
	APTWPlayerState* DeadPlayerState = nullptr;

	if (const APawn* DeadPawn = Cast<APawn>(DeadActor))
	{
		DeadPlayerController = DeadPawn->GetController<APTWPlayerController>();
		DeadPlayerState = DeadPawn->GetPlayerState<APTWPlayerState>();
	}

	if (DeadPlayerState)
	{
		DeadPlayerState->AddDeathCount();
	}

	APTWPlayerState* KillPlayerState = nullptr;

	if (IsValid(KillActor))
	{
		if (APawn* KillPawn = Cast<APawn>(KillActor))
		{
			KillPlayerState = KillPawn->GetPlayerState<APTWPlayerState>();
		}
	}
	
	if (IsValid(KillPlayerState))
	{
		KillPlayerState->AddKillCount();
		KillPlayerState->AddScore(1);
	}
	
	if (!PTWGameState) return;

	PTWGameState->UpdateRanking();

	
	if (IsValid(DeadPlayerController))
	{
		GetWorldTimerManager().ClearTimer(DeadPlayerController->RespawnTimerHandle);		// 방어 코드
		TWeakObjectPtr<ThisClass> WeakThis = this;
		TWeakObjectPtr<APTWPlayerController> WeakDeadController = DeadPlayerController;
		GetWorldTimerManager().SetTimer(WeakDeadController->RespawnTimerHandle, [WeakThis, WeakDeadController]()
		{
			if (WeakThis.IsValid() && WeakDeadController.IsValid())		// 파괴 방어
			{
				WeakThis->RestartPlayer(WeakDeadController.Get());
			}
		}, 5.0f, false);
	}
}

void APTWMiniGameMode::ResetPlayerRoundData()
{
	if (!PTWGameState) return;
	for (APlayerState* PlayerState : PTWGameState->PlayerArray)
	{
		if (APTWPlayerState* PTWPlayerState = Cast<APTWPlayerState>(PlayerState))
		{
			PTWPlayerState->ResetPlayerRoundData();
		}
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

void APTWMiniGameMode::ApplyMiniGameRankScore()
{
	// 현재 랭킹을 기준으로 승리 포인트 추가

	if (!IsValid(PTWGameState)) return;

	// 승리 포인트는 임시로 플레이어 인원 수만큼 지급
	// 동점 계산 X
	for (int i = 0; i < PTWGameState->GetRankedPlayers().Num(); i++)
	{
		FPTWPlayerData PlayerData = PTWGameState->GetRankedPlayers()[i]->GetPlayerData();
		PlayerData.TotalWinPoints += PTWGameState->GetRankedPlayers().Num() - i;
		PTWGameState->GetRankedPlayers()[i]->SetPlayerData(PlayerData);
	}
}

void APTWMiniGameMode::AddWinPoint(APawn* PointPawn, int32 AddPoint)
{
	if (APTWPlayerState* PTWPlayerState = PointPawn->GetPlayerState<APTWPlayerState>())
	{
		FPTWPlayerData PlayerData = PTWPlayerState->GetPlayerData();
		PlayerData.TotalWinPoints += AddPoint;
		PTWPlayerState->SetPlayerData(PlayerData);
	}
}


