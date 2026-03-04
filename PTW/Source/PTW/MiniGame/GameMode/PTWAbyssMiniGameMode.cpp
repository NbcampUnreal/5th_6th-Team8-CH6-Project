// Fill out your copyright notice in the Description page of Project Settings.

#include "MiniGame/GameMode/PTWAbyssMiniGameMode.h"
#include "PTW/System/PTWItemSpawnManager.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/PTWPlayerController.h"
#include "PTWGameplayTag/GameplayTags.h"
#include "EngineUtils.h"
#include "Engine/PostProcessVolume.h"

APTWAbyssMiniGameMode::APTWAbyssMiniGameMode()
{
	AbyssDefaultWeaponTag = GameplayTags::Weapon::Gun::Rifle::Rifle;
}

void APTWAbyssMiniGameMode::StartRound()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APTWPlayerController* PC = Cast<APTWPlayerController>(It->Get()))
		{
			PC->Client_SetAbyssDark(true);
		}
	}

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
		//UE_LOG(LogTemp, Warning, TEXT("[AbyssMiniGameMode] ItemDefinition is NULL. Set it in BP."));
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
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APTWPlayerController* PC = Cast<APTWPlayerController>(It->Get()))
		{
			PC->Client_SetAbyssDark(false);
		}
	}

	GetWorldTimerManager().ClearTimer(CoinSpawnTimerHandle);

	Super::EndRound();
}

void APTWAbyssMiniGameMode::CacheAbyssPP()
{
	if (AbyssPP) return;
	if (!GetWorld()) return;

	for (TActorIterator<APostProcessVolume> It(GetWorld()); It; ++It)
	{
		APostProcessVolume* PP = *It;
		if (!PP) continue;

		if (PP->ActorHasTag(FName("AbyssPP")))
		{
			AbyssPP = PP;
			break;
		}
	}
}

void APTWAbyssMiniGameMode::SetAbyssDark(bool bEnable)
{
	if (!GetWorld()) return;

	CacheAbyssPP();
	if (!AbyssPP) return;
	
	AbyssPP->bEnabled = true;
	AbyssPP->BlendWeight = bEnable ? 1.0f : 0.0f;
}
