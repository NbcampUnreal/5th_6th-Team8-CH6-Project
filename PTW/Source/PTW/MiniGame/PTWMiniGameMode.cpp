// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWMiniGameMode.h"

#include "PTW/System/PTWItemSpawnManager.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
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

void APTWMiniGameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);

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
	if (APTWBaseCharacter* DeadCharacter = Cast<APTWBaseCharacter>(DeadActor))
	{
		if (APTWPlayerState* DeadPlayerState = Cast<APTWPlayerState>(DeadCharacter))
		{
			DeadPlayerState->AddDeathCount();
		}
	}

	if (APTWBaseCharacter* KillCharacter = Cast<APTWBaseCharacter>(KillActor))
	{
		if (APTWPlayerState* KillPlayerState = Cast<APTWPlayerState>(KillCharacter))
		{
			KillPlayerState->AddKillCount();
			KillPlayerState->AddScore(1);
		}
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

void APTWMiniGameMode::AddWinPoint(APawn* PointPawn, int32 AddPoint)
{
	if (APTWPlayerState* PTWPlayerState = PointPawn->GetPlayerState<APTWPlayerState>())
	{
		FPTWPlayerData PlayerData;
		PlayerData.TotalWinPoints += AddPoint;
		PTWPlayerState->SetPlayerData(PlayerData);
	}
}


