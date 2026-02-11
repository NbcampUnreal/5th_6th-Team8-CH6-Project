// Fill out your copyright notice in the Description page of Project Settings.

#include "PTWBombMiniGameMode.h"

#include "GameplayEffectTypes.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "CoreFramework/PTWPlayerState.h"
#include "PTW/MiniGame/Item/BombItem/PTWBombActor.h"

#include "CoreFramework/PTWBaseCharacter.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Inventory/PTWInventoryComponent.h"
#include "System/PTWItemSpawnManager.h"

class UPTWItemSpawnManager;

void APTWBombMiniGameMode::BeginPlay()
{
	Super::BeginPlay();

	CurrentRound = 0;
	
	EliminatedPlayers.Empty();

	//StartRound();
}

// void APTWBombMiniGameMode::StartRound()
// {
// 	CurrentRound++;
//
// 	UE_LOG(LogTemp, Warning, TEXT("[BombMode] Round %d / %d - Countdown Start"), CurrentRound, MaxRoundCount);
//
// 	if (APTWGameState* GS = GetGameState<APTWGameState>())
// 	{
// 		GS->SetbMiniGameCountdown(true);
// 	}
//
// 	StartCountDown();
// }

void APTWBombMiniGameMode::OnCountDownFinished()
{
	Super::OnCountDownFinished();
	
	if (APTWGameState* GS = GetGameState<APTWGameState>())
	{
		GS->SetbMiniGameCountdown(false);
	}

	AssignRandomBombOwner();

	if (!BombActor && BombActorClass)
	{
		BombActor = GetWorld()->SpawnActor<APTWBombActor>(
			BombActorClass,
			FVector::ZeroVector,
			FRotator::ZeroRotator
		);
	}

	if (BombActor && BombOwnerPS)
	{
		APawn* OwnerPawn = BombOwnerPS->GetPawn();
		BombActor->SetBombOwner(OwnerPawn);
	}

	UE_LOG(LogTemp, Warning, TEXT("[BombMode] Round %d - Play Start"), CurrentRound);

	// 라운드 진행 타이머 시작
	//GetWorldTimerManager().SetTimer(RoundTimerHandle, this, &APTWBombMiniGameMode::EndTimer, RoundPlayTime, false);
}

void APTWBombMiniGameMode::EndTimer()
{
	UE_LOG(LogTemp, Warning, TEXT("[BombMode] Round %d - Explosion Timing"), CurrentRound);

	Super::EndTimer();
	
	CleanupBombActor();

	// 3회 다 돌면 미니게임 종료
	// if (CurrentRound >= MaxRoundCount)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("[BombMode] Finished"));
	// 	
	// 	EliminatedPlayers.Empty();
	//
	// 	Super::EndTimer();
	// 	return;
	// }

	// 다음 라운드 진행
	//StartRound();
}

void APTWBombMiniGameMode::CleanupBombActor()
{
	if (BombActor)
	{
		BombActor->Destroy();
		BombActor = nullptr;
	}
}

void APTWBombMiniGameMode::GetAlivePlayerStates(TArray<APTWPlayerState*>& OutAlive) const
{
	OutAlive.Reset();

	const AGameStateBase* GSBase = GetGameState<AGameStateBase>();
	if (!GSBase) return;

	for (APlayerState* PS : GSBase->PlayerArray)
	{
		APTWPlayerState* PTWPS = Cast<APTWPlayerState>(PS);
		if (!PTWPS) continue;
		
		if (EliminatedPlayers.Contains(PTWPS)) continue;
		
		if (PTWPS->IsOnlyASpectator()) continue;
		if (PTWPS->IsInactive()) continue;

		OutAlive.Add(PTWPS);
	}
}

void APTWBombMiniGameMode::AssignRandomBombOwner()
{
	TArray<APTWPlayerState*> AlivePlayers;
	GetAlivePlayerStates(AlivePlayers);

	if (AlivePlayers.Num() <= 0)
	{
		BombOwnerPS = nullptr;
		UE_LOG(LogTemp, Warning, TEXT("[BombMode] Round %d - BombOwner assign failed (no alive players)"), CurrentRound);
		return;
	}

	const int32 PickIndex = FMath::RandRange(0, AlivePlayers.Num() - 1);
	BombOwnerPS = AlivePlayers[PickIndex];
	
	GiveItemAndEquipWeapon();
	
	const FString OwnerName = BombOwnerPS ? BombOwnerPS->GetPlayerName() : TEXT("None");
	UE_LOG(LogTemp, Warning, TEXT("[BombMode] Round %d - BombOwner = %s"), CurrentRound, *OwnerName);
}

void APTWBombMiniGameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);

	// 스폰된 캐릭터 가져오기
	APTWBaseCharacter* BaseChar = Cast<APTWBaseCharacter>(NewPlayer ? NewPlayer->GetPawn() : nullptr);
	if (!BaseChar) return;
	
	BaseChar->OnCharacterDied.RemoveAll(this);
	BaseChar->OnCharacterDied.AddDynamic(this, &APTWBombMiniGameMode::HandleBombPlayerDeath);

	// 이미 탈락한 플레이어는 라운드가 바뀌어도 계속 관전 상태 유지
	if (APlayerState* PS = NewPlayer->PlayerState)
	{
		if (EliminatedPlayers.Contains(PS))
		{
			SetSpectator(NewPlayer);
		}
	}
}

void APTWBombMiniGameMode::GiveItemAndEquipWeapon()
{
	if (!BombOwnerPS) return;
	
	if (APTWPlayerCharacter* PC = Cast<APTWPlayerCharacter>(BombOwnerPS->GetPawn()))
	{
		if (UPTWItemSpawnManager* SpawnManager = GetWorld()->GetSubsystem<UPTWItemSpawnManager>())
		{
			if (IPTWCombatInterface* CombatInt = Cast<IPTWCombatInterface>(PC))
			{
				CombatInt->ApplyGameplayEffectToSelf(BombAttachEffect, 1.0f, FGameplayEffectContextHandle());
			}
			
			SpawnManager->SpawnSingleItem(BombOwnerPS, BombWeaponDef);
			
			if (UPTWInventoryComponent* Inven = PC->GetInventoryComponent())
			{
				Inven->EquipWeapon(0);
			}
		}
	}
}

void APTWBombMiniGameMode::HandleBombPlayerDeath(AActor* Victim, AActor* Attacker)
{
	if (!HasAuthority() || !Victim) return;

	APTWBaseCharacter* VictimChar = Cast<APTWBaseCharacter>(Victim);
	if (!VictimChar) return;

	AController* DeadController = VictimChar->GetController();
	APlayerState* DeadPS = VictimChar->GetPlayerState();
	
	if (APTWGameState* GS = GetGameState<APTWGameState>())
	{
		if (DeadPS && BombOwnerPS)
		{
			GS->Multicast_BroadcastKilllogEx(
				Cast<AActor>(DeadPS),          // Victim
				Cast<AActor>(BombOwnerPS),    // Killer 
				FName(TEXT("BOMB"))            // 원인
			);
		}
	}

	if (DeadPS)
	{
		EliminatedPlayers.Add(DeadPS);
	}

	if (DeadController)
	{
		SetSpectator(DeadController);
	}
}


void APTWBombMiniGameMode::SetSpectator(AController* DeadController)
{
	APlayerController* PC = Cast<APlayerController>(DeadController);
	if (!PC) return;

	PC->ChangeState(NAME_Spectating);
	PC->StartSpectatingOnly();
}
