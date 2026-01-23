// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWPlayerController.h"
#include "UI/PTWHUD.h"
#include "PTWPlayerState.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "UI/RankBoard/PTWRankingBoard.h"

void APTWPlayerController::BeginPlay()
{
	Super::BeginPlay();

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
	if (RankingBoardClass)
	{
		RankingBoard = CreateWidget<UPTWRankingBoard>(this, RankingBoardClass);

		if (RankingBoard)
		{
			RankingBoard->AddToViewport();
			RankingBoard->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void APTWPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	TryInitializeHUD();
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
	}
}

void APTWPlayerController::OnRankingPressed()
{
	if (!RankingBoard) return;

	RankingBoard->SetVisibility(ESlateVisibility::Visible);

	// 마우스 + UI 입력 허용
	bShowMouseCursor = true;
	SetInputMode(FInputModeGameAndUI());
}

void APTWPlayerController::OnRankingReleased()
{
	if (!RankingBoard) return;

	RankingBoard->SetVisibility(ESlateVisibility::Hidden);

	// 게임 입력으로 복귀
	bShowMouseCursor = false;
	SetInputMode(FInputModeGameOnly());
}
