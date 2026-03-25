// Fill out your copyright notice in the Description page of Project Settings.

#include "MiniGame/GameMode/PTWAbyssMiniGameMode.h"

#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/PTWPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "AbilitySystemComponent.h"
#include "PTWGameplayTag/GameplayTags.h"

APTWAbyssMiniGameMode::APTWAbyssMiniGameMode()
{
}

void APTWAbyssMiniGameMode::StartRound()
{
	bIsBlackoutActive = false;
	ApplyBlackoutState(false);

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

		GetWorldTimerManager().ClearTimer(BlackoutTimerHandle);
		GetWorldTimerManager().ClearTimer(BlackoutEndTimerHandle);

		if (bUseBlackoutCycle)
		{
			ScheduleBlackout();
		}
	}
}

void APTWAbyssMiniGameMode::EndRound()
{
	bIsBlackoutActive = false;
	ApplyBlackoutState(false);

	GetWorldTimerManager().ClearTimer(CoinSpawnTimerHandle);

	if (HasAuthority())
	{
		GetWorldTimerManager().ClearTimer(IdleRevealTimerHandle);
		GetWorldTimerManager().ClearTimer(BlackoutTimerHandle);
		GetWorldTimerManager().ClearTimer(BlackoutEndTimerHandle);

		ClearAllRevealMarkers();
		IdleTimeMap.Empty();
	}

	Super::EndRound();
}

void APTWAbyssMiniGameMode::ApplyBlackoutState(bool bEnable)
{
	if (!GetWorld()) return;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APTWPlayerController* PC = Cast<APTWPlayerController>(It->Get()))
		{
			PC->Client_SetAbyssDark(bEnable);

			if (APTWPlayerCharacter* Character = Cast<APTWPlayerCharacter>(PC->GetPawn()))
			{
				Character->SetStealthMode(bEnable);

				if (APTWPlayerState* PS = Character->GetPlayerState<APTWPlayerState>())
				{
					if (UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
					{
						if (bEnable)
						{
							ASC->RemoveLooseGameplayTag(GameplayTags::State::Abyss::NoFire);
						}
						else
						{
							ASC->AddLooseGameplayTag(GameplayTags::State::Abyss::NoFire);
						}
					}
				}
			}
		}
	}

	if (!bEnable)
	{
		ClearAllRevealMarkers();
		IdleTimeMap.Empty();
	}
}

void APTWAbyssMiniGameMode::ScheduleBlackout()
{
	if (!HasAuthority() || !GetWorld()) return;
	if (!bUseBlackoutCycle) return;

	const float NextInterval = FMath::FRandRange(BlackoutMinInterval, BlackoutMaxInterval);

	GetWorldTimerManager().SetTimer(
		BlackoutTimerHandle,
		this,
		&ThisClass::StartBlackout,
		NextInterval,
		false
	);
}

void APTWAbyssMiniGameMode::StartBlackout()
{
	if (!HasAuthority() || !GetWorld()) return;

	bIsBlackoutActive = true;
	ApplyBlackoutState(true);

	GetWorldTimerManager().SetTimer(
		BlackoutEndTimerHandle,
		this,
		&ThisClass::EndBlackout,
		BlackoutDuration,
		false
	);
}

void APTWAbyssMiniGameMode::EndBlackout()
{
	if (!HasAuthority() || !GetWorld()) return;

	bIsBlackoutActive = false;
	ApplyBlackoutState(false);

	if (bUseBlackoutCycle)
	{
		ScheduleBlackout();
	}
}

void APTWAbyssMiniGameMode::TickIdleReveal()
{
	if (!HasAuthority() || !GetWorld()) return;

	if (!bIsBlackoutActive)
	{
		ClearAllRevealMarkers();
		IdleTimeMap.Empty();
		return;
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AController* Controller = It->Get();
		if (!Controller) continue;

		APawn* Pawn = Controller->GetPawn();
		APlayerState* PS = Controller->PlayerState;
		if (!Pawn || !PS) continue;

		const float Speed = Pawn->GetVelocity().Size();
		float& Acc = IdleTimeMap.FindOrAdd(PS);

		if (Speed <= IdleSpeedThreshold)
		{
			Acc += IdleCheckInterval;

			if (Acc >= IdleRevealTime)
			{
				ShowReveal(Controller);
			}
		}
		else
		{
			Acc = 0.0f;
			HideReveal(Controller);
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

	RevealMarkerMap.Add(PS, Marker);
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

void APTWAbyssMiniGameMode::ClearAllRevealMarkers()
{
	for (auto& Pair : RevealMarkerMap)
	{
		if (IsValid(Pair.Value))
		{
			Pair.Value->Destroy();
		}
	}

	RevealMarkerMap.Empty();
}

void APTWAbyssMiniGameMode::HandlePlayerDeath(AActor* DeadActor, AActor* KillActor)
{
	Super::HandlePlayerDeath(DeadActor, KillActor);

	if (!PTWGameState) return;

	PTWGameState->UpdateRanking(MiniGameRule);
	CheckEndGameCondition();
}
