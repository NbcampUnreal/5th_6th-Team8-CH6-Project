// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWPlayerController.h"
#include "UI/PTWHUD.h"
#include "PTWPlayerState.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/GameState.h"
#include "UI/RankBoard/PTWRankingBoard.h"

void APTWPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}

	UE_LOG(LogTemp, Error, TEXT("Controller BeginPlay"));
	TryInitializeHUD();

	/* Input Mapping Context 추가 */
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	/* 랭킹보드 위젯 생성 */
	CreateRankingBoard();
}

void APTWPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (!IsLocalController())
	{
		return;
	}

	TryInitializeHUD();
}

void APTWPlayerController::BeginSpectatingState()
{
	Super::BeginSpectatingState();
	
	if (IsLocalController() && !IsValid(GetPawn()))
	{
		SpectateNextPlayer();
	}
}

void APTWPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();
	
	if (IsLocalController() && !IsValid(GetPawn()))
	{
		SpectateNextPlayer();
	}
}

void APTWPlayerController::TryInitializeHUD()
{
	UE_LOG(LogTemp, Error, TEXT("Controller TryInitializeHUD"));
	// HUD 확인
	TObjectPtr<APTWHUD> PTWHUD = Cast<APTWHUD>(GetHUD());
	if (!PTWHUD)
	{
		return;
	}

	// PlayerState 확인
	TObjectPtr<APTWPlayerState> PTWPS = GetPlayerState<APTWPlayerState>();
	if (!PTWPS)
	{
		return;
	}

	// ASC 확인
	TObjectPtr<UAbilitySystemComponent> ASC = PTWPS->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	// HUD 초기화
	PTWHUD->InitializeHUD(ASC);
}

void APTWPlayerController::StartSpectating()
{
	if (HasAuthority())
	{
		ChangeState(NAME_Spectating);
		ClientGotoState(NAME_Spectating);
	}
}

void APTWPlayerController::SpectateNextPlayer()
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}
	
	AGameStateBase* GS = World->GetGameState();
	if (!IsValid(GS))
	{
		return;
	}
	
	const TArray<APlayerState*>& PlayArray = GS->PlayerArray;
	if (PlayArray.IsEmpty())
	{
		return;
	}
	
	AActor* CurrentViewTarget = GetViewTarget();
	if (!IsValid(CurrentViewTarget))
	{
		return;
	}

	APlayerState* CurrentPlayerState = nullptr;
	if (APawn* CastPawn = Cast<APawn>(CurrentViewTarget))
	{
		CurrentPlayerState = CastPawn->GetPlayerState();
	}
	
	APawn* NewViewTarget = nullptr;
	if (IsValid(CurrentPlayerState))
	{
		int32 FoundIndex = PlayArray.Find(CurrentPlayerState);
		if (FoundIndex != INDEX_NONE)
		{
			for (int32 i = FoundIndex + 1; i < PlayArray.Num(); i++)
			{
				if (PlayArray[i]->GetPawn()->GetPlayerState() && !PlayArray[i]->IsSpectator())
				{
					NewViewTarget = PlayArray[i]->GetPawn();
					break;
				}
			}
		}
	}
	
	if (!IsValid(NewViewTarget))
	{
		for (const APlayerState* PS : PlayArray)
		{
			if (PS != PlayerState && !PS->IsSpectator() && PS->GetPawn()->GetPlayerState())
			{
				NewViewTarget = PS->GetPawn();
				break;
			}
		}
	}
	
	if (IsValid(NewViewTarget))
	{
		SetViewTargetWithBlend(NewViewTarget, 0.5f, VTBlend_Cubic);
	}
}

void APTWPlayerController::OnInputSpectateNext()
{
	if (GetStateName() == NAME_Spectating)
	{
		SpectateNextPlayer();
	}
}

void APTWPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EIC->BindAction(
			ShowRankingAction,
			ETriggerEvent::Started,
			this,
			&APTWPlayerController::OnRankingPressed
		);

		EIC->BindAction(
			ShowRankingAction,
			ETriggerEvent::Completed,
			this,
			&APTWPlayerController::OnRankingReleased
		);
		EIC->BindAction(
			SpectateNextAction, 
			ETriggerEvent::Started, 
			this, 
			&ThisClass::OnInputSpectateNext);
	}
}

void APTWPlayerController::OnRankingPressed()
{
	if (!RankingBoard) return;

	RankingBoard->SetVisibility(ESlateVisibility::Visible);
}

void APTWPlayerController::OnRankingReleased()
{
	if (!RankingBoard) return;

	RankingBoard->SetVisibility(ESlateVisibility::Hidden);
}

void APTWPlayerController::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();

	if (!IsLocalController())
	{
		return;
	}

	CreateRankingBoard();
}

void APTWPlayerController::CreateRankingBoard()
{
	if (RankingBoard)
	{
		RankingBoard->RemoveFromParent();
		RankingBoard = nullptr;
	}

	if (!RankingBoardClass)
	{
		return;
	}

	RankingBoard = CreateWidget<UPTWRankingBoard>(this, RankingBoardClass);

	if (RankingBoard)
	{
		RankingBoard->AddToViewport();
		RankingBoard->SetVisibility(ESlateVisibility::Hidden);
	}
}
