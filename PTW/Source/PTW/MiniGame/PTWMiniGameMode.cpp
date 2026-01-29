// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWMiniGameMode.h"

#include "PTW/System/PTWItemSpawnManager.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "GAS/PTWAttributeSet.h"
#include "System/PTWScoreSubsystem.h"
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
	
	StartTimer(MiniGameTime);
}

void APTWMiniGameMode::EndTimer()
{
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
	Super::RestartPlayer(NewPlayer);
	
	InitPlayerHealth(NewPlayer);
	SpawnDefaultWeapon(NewPlayer);
	
	if (APTWBaseCharacter* BaseCharacter = Cast<APTWPlayerCharacter>(NewPlayer->GetPawn()))
	{
		BaseCharacter->OnCharacterDied.AddDynamic(this, &APTWMiniGameMode::HandlePlayerDeath);
		UE_LOG(LogTemp, Warning, TEXT("RestartPlayer"));
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
	APTWPlayerController* DeadPlayerController = nullptr;
	if (APTWBaseCharacter* DeadCharacter = Cast<APTWBaseCharacter>(DeadActor))
	{
		DeadPlayerController = DeadCharacter->GetController<APTWPlayerController>();
		if (APTWPlayerState* DeadPlayerState = Cast<APTWPlayerState>(DeadCharacter->GetPlayerState()))
		{
			DeadPlayerState->AddDeathCount();
		}
	}

	if (APTWPlayerState* KillPlayerState = Cast<APTWPlayerState>(KillActor))
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

void APTWMiniGameMode::AddWinPoint(APawn* PointPawn, int32 AddPoint)
{
	if (APTWPlayerState* PTWPlayerState = PointPawn->GetPlayerState<APTWPlayerState>())
	{
		FPTWPlayerData PlayerData;
		PlayerData.TotalWinPoints += AddPoint;
		PTWPlayerState->SetPlayerData(PlayerData);
	}
}


