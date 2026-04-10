// Fill out your copyright notice in the Description page of Project Settings.

#include "MiniGame/GameMode/PTWAbyssMiniGameMode.h"

#include "AbilitySystemComponent.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/PTWPlayerState.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "PTWGameplayTag/GameplayTags.h"

#define LOCTEXT_NAMESPACE "PTWAbyssMiniGameMode"

APTWAbyssMiniGameMode::APTWAbyssMiniGameMode()
{
}

void APTWAbyssMiniGameMode::StartCountDown()
{
	Super::StartCountDown();
	
	ApplyBlackoutStateToAllPlayers(false);
}

void APTWAbyssMiniGameMode::StartRound()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APTWPlayerController* PC = Cast<APTWPlayerController>(It->Get()))
		{
			PC->SendMessage(
				LOCTEXT("AbyssRoundStartMessage", "어둠이 찾아오면 적을 찾아 잡으세요!"),
				ENotificationPriority::High,
				3.f
			);
		}
	}

	bIsBlackoutActive = false;
	ApplyBlackoutStateToAllPlayers(false);

	if (MiniGameRule.TimeRule.bUseTimer)
	{
		StartTimer(MiniGameRule.TimeRule.Timer);
	}

	StartChaosEvent();

	if (HasAuthority())
	{
		StartIdleRevealTracking();
		StartBlackoutCycle();
	}
}

void APTWAbyssMiniGameMode::EndRound()
{
	bIsBlackoutActive = false;
	ApplyBlackoutStateToAllPlayers(false);

	GetWorldTimerManager().ClearTimer(CoinSpawnTimerHandle);

	if (HasAuthority())
	{
		StopIdleRevealTracking();
		StopBlackoutCycle();
		ResetIdleRevealState();
	}

	Super::EndRound();
}

void APTWAbyssMiniGameMode::HandlePlayerDeath(AActor* DeadActor, AActor* KillActor)
{
	Super::HandlePlayerDeath(DeadActor, KillActor);

	if (!PTWGameState)
	{
		return;
	}

	PTWGameState->UpdateRanking(MiniGameRule);
	CheckEndGameCondition();
}

void APTWAbyssMiniGameMode::HandleRespawn(APTWPlayerController* PlayerController)
{
	Super::HandleRespawn(PlayerController);

	if (!IsValid(PlayerController))
	{
		return;
	}

	ApplyBlackoutStateToPlayer(PlayerController, bIsBlackoutActive);
}

void APTWAbyssMiniGameMode::StartBlackoutCycle()
{
	GetWorldTimerManager().ClearTimer(BlackoutTimerHandle);
	GetWorldTimerManager().ClearTimer(BlackoutEndTimerHandle);

	if (!bUseBlackoutCycle)
	{
		return;
	}

	ScheduleNextBlackout();
}

void APTWAbyssMiniGameMode::StopBlackoutCycle()
{
	GetWorldTimerManager().ClearTimer(BlackoutTimerHandle);
	GetWorldTimerManager().ClearTimer(BlackoutEndTimerHandle);
}

void APTWAbyssMiniGameMode::ScheduleNextBlackout()
{
	if (!HasAuthority() || !GetWorld())
	{
		return;
	}

	if (!bUseBlackoutCycle)
	{
		return;
	}

	const float NextInterval = FMath::FRandRange(BlackoutMinInterval, BlackoutMaxInterval);

	GetWorldTimerManager().SetTimer(
		BlackoutTimerHandle,
		this,
		&ThisClass::BeginBlackout,
		NextInterval,
		false
	);
}

void APTWAbyssMiniGameMode::BeginBlackout()
{
	if (!HasAuthority() || !GetWorld())
	{
		return;
	}

	bIsBlackoutActive = true;
	ApplyBlackoutStateToAllPlayers(true);

	GetWorldTimerManager().SetTimer(
		BlackoutEndTimerHandle,
		this,
		&ThisClass::FinishBlackout,
		BlackoutDuration,
		false
	);
}

void APTWAbyssMiniGameMode::FinishBlackout()
{
	if (!HasAuthority() || !GetWorld())
	{
		return;
	}

	bIsBlackoutActive = false;
	ApplyBlackoutStateToAllPlayers(false);

	if (bUseBlackoutCycle)
	{
		ScheduleNextBlackout();
	}
}

void APTWAbyssMiniGameMode::ApplyBlackoutStateToAllPlayers(bool bEnable)
{
	if (!GetWorld())
	{
		return;
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APTWPlayerController* PC = Cast<APTWPlayerController>(It->Get()))
		{
			ApplyBlackoutStateToPlayer(PC, bEnable);
		}
	}

	if (!bEnable)
	{
		ResetIdleRevealState();
	}
}

void APTWAbyssMiniGameMode::ApplyBlackoutStateToPlayer(APTWPlayerController* PC, bool bEnable)
{
	if (!IsValid(PC))
	{
		return;
	}

	ApplyBlackoutVisual(PC, bEnable);
	ApplyBlackoutStealth(PC, bEnable);
	ApplyBlackoutFireRestriction(PC, bEnable);
}

void APTWAbyssMiniGameMode::ApplyBlackoutVisual(APTWPlayerController* PC, bool bEnable)
{
	if (!IsValid(PC))
	{
		return;
	}

	PC->Client_SetAbyssDark(bEnable);
}

void APTWAbyssMiniGameMode::ApplyBlackoutStealth(APTWPlayerController* PC, bool bEnable)
{
	if (!IsValid(PC))
	{
		return;
	}

	APTWPlayerCharacter* Character = Cast<APTWPlayerCharacter>(PC->GetPawn());
	if (!IsValid(Character))
	{
		return;
	}

	Character->SetStealthMode(bEnable);
}

void APTWAbyssMiniGameMode::ApplyBlackoutFireRestriction(APTWPlayerController* PC, bool bEnable)
{
	if (!IsValid(PC))
	{
		return;
	}

	APTWPlayerCharacter* Character = Cast<APTWPlayerCharacter>(PC->GetPawn());
	if (!IsValid(Character))
	{
		return;
	}

	APTWPlayerState* PS = Character->GetPlayerState<APTWPlayerState>();
	if (!IsValid(PS))
	{
		return;
	}

	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	if (!IsValid(ASC))
	{
		return;
	}

	if (bEnable)
	{
		ASC->RemoveLooseGameplayTag(GameplayTags::State::Abyss::NoFire);
	}
	else
	{
		ASC->AddLooseGameplayTag(GameplayTags::State::Abyss::NoFire);
	}
}

void APTWAbyssMiniGameMode::StartIdleRevealTracking()
{
	GetWorldTimerManager().ClearTimer(IdleRevealTimerHandle);

	GetWorldTimerManager().SetTimer(
		IdleRevealTimerHandle,
		this,
		&ThisClass::UpdateIdleReveal,
		IdleCheckInterval,
		true
	);
}

void APTWAbyssMiniGameMode::StopIdleRevealTracking()
{
	GetWorldTimerManager().ClearTimer(IdleRevealTimerHandle);
}

void APTWAbyssMiniGameMode::UpdateIdleReveal()
{
	if (!HasAuthority() || !GetWorld())
	{
		return;
	}

	if (!bIsBlackoutActive)
	{
		ResetIdleRevealState();
		return;
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AController* Controller = It->Get();
		if (!IsValid(Controller))
		{
			continue;
		}

		APawn* Pawn = Controller->GetPawn();
		APlayerState* PS = Controller->PlayerState;
		if (!IsValid(Pawn) || !IsValid(PS))
		{
			continue;
		}

		const float Speed = Pawn->GetVelocity().Size();
		float& IdleAccumulatedTime = IdleTimeMap.FindOrAdd(PS);

		if (Speed <= IdleSpeedThreshold)
		{
			IdleAccumulatedTime += IdleCheckInterval;

			if (IdleAccumulatedTime >= IdleRevealTime)
			{
				ShowReveal(Controller);
			}
		}
		else
		{
			IdleAccumulatedTime = 0.0f;
			HideReveal(Controller);
		}
	}
}

void APTWAbyssMiniGameMode::ResetIdleRevealState()
{
	ClearAllRevealMarkers();
	IdleTimeMap.Empty();
}

void APTWAbyssMiniGameMode::ShowReveal(AController* Controller)
{
	if (!IsValid(Controller) || !RevealMarkerClass || !GetWorld())
	{
		return;
	}

	APlayerState* PS = Controller->PlayerState;
	APawn* Pawn = Controller->GetPawn();
	if (!IsValid(PS) || !IsValid(Pawn))
	{
		return;
	}

	if (TObjectPtr<AActor>* FoundMarker = RevealMarkerMap.Find(PS))
	{
		if (IsValid(*FoundMarker))
		{
			return;
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Pawn;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* Marker = GetWorld()->SpawnActor<AActor>(
		RevealMarkerClass,
		Pawn->GetActorLocation(),
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (!IsValid(Marker))
	{
		return;
	}

	RevealMarkerMap.Add(PS, Marker);
}

void APTWAbyssMiniGameMode::HideReveal(AController* Controller)
{
	if (!IsValid(Controller))
	{
		return;
	}

	APlayerState* PS = Controller->PlayerState;
	if (!IsValid(PS))
	{
		return;
	}

	if (TObjectPtr<AActor>* FoundMarker = RevealMarkerMap.Find(PS))
	{
		if (IsValid(*FoundMarker))
		{
			(*FoundMarker)->Destroy();
		}

		RevealMarkerMap.Remove(PS);
	}
}

void APTWAbyssMiniGameMode::ClearAllRevealMarkers()
{
	for (TPair<TObjectPtr<APlayerState>, TObjectPtr<AActor>>& Pair : RevealMarkerMap)
	{
		if (IsValid(Pair.Value))
		{
			Pair.Value->Destroy();
		}
	}

	RevealMarkerMap.Empty();
}

#undef LOCTEXT_NAMESPACE
