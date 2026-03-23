// Fill out your copyright notice in the Description page of Project Settings.

#include "MiniGame/GameMode/PTWAbyssMiniGameMode.h"

#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/PTWPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "System/PTWItemSpawnManager.h"
#include "CoreFramework/Character/Component/PTWWeaponComponent.h"
#include "Inventory/PTWInventoryComponent.h"
#include "Inventory/Instance/PTWWeaponInstance.h"
#include "PTW/Inventory/PTWItemDefinition.h"

APTWAbyssMiniGameMode::APTWAbyssMiniGameMode()
{
}

void APTWAbyssMiniGameMode::StartRound()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APTWPlayerController* PC = Cast<APTWPlayerController>(It->Get()))
		{
			PC->Client_SetAbyssDark(true);

			if (APTWPlayerCharacter* Character = Cast<APTWPlayerCharacter>(PC->GetPawn()))
			{
				Character->SetStealthMode(true);
			}
		}
	}

	if (MiniGameRule.TimeRule.bUseTimer)
	{
		StartTimer(MiniGameRule.TimeRule.Timer);
	}

	StartChaosEvent();

	if (HasAuthority())
	{
		GetWorldTimerManager().ClearTimer(IdleRevealTimerHandle);
		GetWorldTimerManager().SetTimer(
			IdleRevealTimerHandle,
			this,
			&ThisClass::TickIdleReveal,
			IdleCheckInterval,
			true
		);

		if (bUseLightningFlash)
		{
			ScheduleLightningFlash();
		}
	}
}

void APTWAbyssMiniGameMode::EndRound()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APTWPlayerController* PC = Cast<APTWPlayerController>(It->Get()))
		{
			PC->Client_SetAbyssDark(false);

			if (APTWPlayerCharacter* Character = Cast<APTWPlayerCharacter>(PC->GetPawn()))
			{
				Character->SetStealthMode(false);
			}
		}
	}

	GetWorldTimerManager().ClearTimer(CoinSpawnTimerHandle);

	if (HasAuthority())
	{
		GetWorldTimerManager().ClearTimer(IdleRevealTimerHandle);
		GetWorldTimerManager().ClearTimer(LightningTimerHandle);
		GetWorldTimerManager().ClearTimer(LightningRestoreTimerHandle);

		for (auto& Pair : RevealMarkerMap)
		{
			if (Pair.Value)
			{
				Pair.Value->Destroy();
			}
		}

		RevealMarkerMap.Empty();
		IdleTimeMap.Empty();
	}

	Super::EndRound();
}

void APTWAbyssMiniGameMode::TickIdleReveal()
{
	if (!HasAuthority() || !GetWorld()) return;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AController* C = It->Get();
		if (!C) continue;

		APawn* Pawn = C->GetPawn();
		APlayerState* PS = C->PlayerState;
		if (!Pawn || !PS) continue;

		const float Speed = Pawn->GetVelocity().Size();
		float& Acc = IdleTimeMap.FindOrAdd(PS);

		if (Speed <= IdleSpeedThreshold)
		{
			Acc += IdleCheckInterval;

			if (Acc >= IdleRevealTime)
			{
				ShowReveal(C);
			}
		}
		else
		{
			Acc = 0.0f;
			HideReveal(C);
		}
	}
}

void APTWAbyssMiniGameMode::ShowReveal(AController* Controller)
{
	if (!Controller || !RevealMarkerClass || !GetWorld()) return;

	APlayerState* PS = Controller->PlayerState;
	APawn* Pawn = Controller->GetPawn();
	if (!PS || !Pawn) return;

	if (TObjectPtr<AActor>* Found = RevealMarkerMap.Find(PS))
	{
		if (IsValid(*Found)) return;
	}

	FActorSpawnParameters Params;
	Params.Owner = Pawn;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* Marker = GetWorld()->SpawnActor<AActor>(
		RevealMarkerClass,
		Pawn->GetActorLocation(),
		FRotator::ZeroRotator,
		Params
	);

	if (!Marker) return;

	RevealMarkerMap.Add(Controller->PlayerState, Marker);
}

void APTWAbyssMiniGameMode::HideReveal(AController* Controller)
{
	if (!Controller) return;

	APlayerState* PS = Controller->PlayerState;
	if (!PS) return;

	if (TObjectPtr<AActor>* Found = RevealMarkerMap.Find(PS))
	{
		if (IsValid(*Found))
		{
			(*Found)->Destroy();
		}
		RevealMarkerMap.Remove(PS);
	}
}

void APTWAbyssMiniGameMode::ScheduleLightningFlash()
{
	if (!HasAuthority() || !GetWorld()) return;

	const float NextInterval = FMath::FRandRange(LightningMinInterval, LightningMaxInterval);

	GetWorldTimerManager().SetTimer(
		LightningTimerHandle,
		this,
		&ThisClass::TriggerLightningFlash,
		NextInterval,
		false
	);
}

void APTWAbyssMiniGameMode::TriggerLightningFlash()
{
	if (!HasAuthority() || !GetWorld()) return;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APTWPlayerController* PC = Cast<APTWPlayerController>(It->Get()))
		{
			PC->Client_SetAbyssDark(false);
		}
	}

	GetWorldTimerManager().SetTimer(
		LightningRestoreTimerHandle,
		this,
		&ThisClass::RestoreAbyssDark,
		LightningFlashDuration,
		false
	);
}

void APTWAbyssMiniGameMode::RestoreAbyssDark()
{
	if (!HasAuthority() || !GetWorld()) return;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APTWPlayerController* PC = Cast<APTWPlayerController>(It->Get()))
		{
			PC->Client_SetAbyssDark(true);
		}
	}

	if (bUseLightningFlash)
	{
		ScheduleLightningFlash();
	}
}

void APTWAbyssMiniGameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);

	if (!NewPlayer) return;

	GiveAndEquipDefaultWeapon(NewPlayer);
}

void APTWAbyssMiniGameMode::GiveAndEquipDefaultWeapon(AController* NewPlayer)
{
	// if (!NewPlayer) return;
	//
	// APTWPlayerCharacter* PlayerCharacter = Cast<APTWPlayerCharacter>(NewPlayer->GetPawn());
	// if (!PlayerCharacter) return;
	//
	// UPTWWeaponComponent* WeaponComp = PlayerCharacter->GetWeaponComponent();
	// UPTWInventoryComponent* InvenComp = PlayerCharacter->GetInventoryComponent();
	// UPTWItemSpawnManager* SpawnManager = GetWorld()->GetSubsystem<UPTWItemSpawnManager>();
	//
	// if (!WeaponComp || !InvenComp || !SpawnManager) return;
	// if (MiniGameRule.LoadoutRule.DefaultWeapon.IsEmpty()) return;
	//
	// for (UPTWItemDefinition* DefaultWeapon : MiniGameRule.LoadoutRule.DefaultWeapon)
	// {
	// 	if (!DefaultWeapon) continue;
	//
	// 	SpawnManager->SpawnWeaponActor(
	// 		PlayerCharacter,
	// 		DefaultWeapon,
	// 		DefaultWeapon->WeaponTag
	// 	);
	// }
	//
	// FWeaponPair WeaponPairs = InvenComp->GetWeaponActorsArr(NewPlayer);
	// if (!WeaponPairs.Weapon1P || !WeaponPairs.Weapon3P) return;
	//
	// UPTWWeaponInstance* WeaponInst = WeaponPairs.Weapon3P->GetWeaponItemInstance();
	// if (!WeaponInst) return;
	//
	// UPTWItemDefinition* Def = WeaponInst->ItemDef;
	// if (!Def) return;
	//
	// WeaponComp->AttachWeaponToSocket(
	// 	WeaponPairs.Weapon1P,
	// 	WeaponPairs.Weapon3P,
	// 	Def->WeaponTag
	// );
	//
	// WeaponComp->EquipWeaponByTag(Def->WeaponTag);
}

void APTWAbyssMiniGameMode::HandlePlayerDeath(AActor* DeadActor, AActor* KillActor)
{
	APTWPlayerController* DeadPlayerController = nullptr;
	APlayerState* DeadPlayerState = nullptr;

	if (const APawn* DeadPawn = Cast<APawn>(DeadActor))
	{
		DeadPlayerController = DeadPawn->GetController<APTWPlayerController>();
		DeadPlayerState = DeadPawn->GetPlayerState();
	}

	APlayerState* KillPlayerState = nullptr;
	if (APawn* KillPawn = Cast<APawn>(KillActor))
	{
		KillPlayerState = KillPawn->GetPlayerState<APlayerState>();
	}
	else
	{
		KillPlayerState = Cast<APlayerState>(KillActor);
	}

	AddKillDeathCount(DeadPlayerState, KillPlayerState);

	if (!PTWGameState) return;

	PTWGameState->UpdateRanking(MiniGameRule);
	PTWGameState->AlivePlayers.Remove(DeadPlayerState);

	CheckEndGameCondition();
	AbyssRespawnPlayer(DeadPlayerController);
}

void APTWAbyssMiniGameMode::AbyssRespawnPlayer(APTWPlayerController* SpawnPlayerController)
{
	if (!MiniGameRule.SpawnRule.bUseRespawn || !SpawnPlayerController) return;

	GetWorldTimerManager().ClearTimer(SpawnPlayerController->RespawnTimerHandle);

	GetWorldTimerManager().SetTimer(
		SpawnPlayerController->RespawnTimerHandle,
		FTimerDelegate::CreateLambda([this, SpawnPlayerController]()
		{
			if (!SpawnPlayerController) return;

			ExitSpectatorMode(SpawnPlayerController);
			RestartPlayer(SpawnPlayerController);

			if (MiniGameRule.SpawnRule.bUseSpawnProtection)
			{
				if (APTWPlayerState* PlayerState = SpawnPlayerController->GetPlayerState<APTWPlayerState>())
				{
					PlayerState->ApplyInvincible(MiniGameRule.SpawnRule.SpawnProtectionTime);
				}
			}
		}),
		MiniGameRule.SpawnRule.RespawnDelay,
		false
	);
}
