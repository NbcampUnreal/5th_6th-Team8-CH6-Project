// Fill out your copyright notice in the Description page of Project Settings.

#include "MiniGame/GameMode/PTWAbyssMiniGameMode.h"
#include "PTW/System/PTWItemSpawnManager.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "PTWGameplayTag/GameplayTags.h"

APTWAbyssMiniGameMode::APTWAbyssMiniGameMode()
{
	AbyssDefaultWeaponTag = GameplayTags::Weapon::Gun::Rifle::Rifle;
}

void APTWAbyssMiniGameMode::StartRound()
{
	if (MiniGameRule.TimeRule.bUseTimer)
	{
		StartTimer(MiniGameRule.TimeRule.Timer);
	}
	
	StartChaosEvent();
	
}

void APTWAbyssMiniGameMode::RestartPlayer(AController* NewPlayer)
{
	if (!NewPlayer) return;
	
	Super::RestartPlayer(NewPlayer);


	GiveAbyssDefaultWeapon(NewPlayer);
}

void APTWAbyssMiniGameMode::GiveAbyssDefaultWeapon(AController* NewPlayer)
{
	if (!NewPlayer) return;

	if (!ItemDefinition)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AbyssMiniGameMode] ItemDefinition is NULL. Set it in BP."));
		return;
	}

	APawn* Pawn = NewPlayer->GetPawn();
	if (!Pawn) return;

	APTWPlayerCharacter* PlayerCharacter = Cast<APTWPlayerCharacter>(Pawn);
	if (!PlayerCharacter) return;

	UPTWItemSpawnManager* ItemSpawnManager = GetWorld() ? GetWorld()->GetSubsystem<UPTWItemSpawnManager>() : nullptr;
	if (!ItemSpawnManager) return;

	ItemSpawnManager->SpawnWeaponActor(PlayerCharacter, ItemDefinition, AbyssDefaultWeaponTag);
}

void APTWAbyssMiniGameMode::EndRound()
{
	GetWorldTimerManager().ClearTimer(CoinSpawnTimerHandle);

	Super::EndRound();
}
