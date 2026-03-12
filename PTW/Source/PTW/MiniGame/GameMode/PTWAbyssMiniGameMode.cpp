// Fill out your copyright notice in the Description page of Project Settings.

#include "MiniGame/GameMode/PTWAbyssMiniGameMode.h"

#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "GameFramework/PlayerController.h"
#include "System/PTWItemSpawnManager.h"
#include "PTWGameplayTag/GameplayTags.h"


APTWAbyssMiniGameMode::APTWAbyssMiniGameMode()
{
	AbyssDefaultWeaponTag = GameplayTags::Weapon::Gun::Pistol::AbyssPistol;
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

void APTWAbyssMiniGameMode::SpawnDefaultWeapon(AController* NewPlayer)
{
	if (!ItemDefinition)
	{
		return;
	}

	if (UPTWItemSpawnManager* ItemSpawnManager = GetWorld()->GetSubsystem<UPTWItemSpawnManager>())
	{
		if (APTWPlayerCharacter* PlayerCharacter = Cast<APTWPlayerCharacter>(NewPlayer->GetPawn()))
		{
			ItemSpawnManager->SpawnWeaponActor(PlayerCharacter, ItemDefinition, AbyssDefaultWeaponTag);
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
