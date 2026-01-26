// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWPlayerController.h"
#include "UI/PTWHUD.h"
#include "PTWPlayerState.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "UI/RankBoard/PTWRankingBoard.h"
#include "CoreFramework/PTWBaseCharacter.h"
#include "Inventory/PTWItemInstance.h"
#include "Inventory/PTWWeaponActor.h"
#include "GameplayTagContainer.h"


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

void APTWPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	//if (APTWBaseCharacter* PTWCharacter = Cast<APTWBaseCharacter>(InPawn))
	//{
	//	//캐릭터의 무기 변경 이벤트 구독 
	//	 ex) PTWCharacter->OnWeaponChanged.AddUObject(this, &ThisClass::HandleWeaponChanged);
	//}
}

void APTWPlayerController::OnUnPossess()
{
	UnbindAmmoDelegate();
	CurrentWeaponItem = nullptr;

	Super::OnUnPossess();
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

	RankingBoard->UpdateRanking();
	RankingBoard->SetVisibility(ESlateVisibility::Visible);
}

void APTWPlayerController::OnRankingReleased()
{
	if (!RankingBoard) return;

	RankingBoard->SetVisibility(ESlateVisibility::Hidden);
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

void APTWPlayerController::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();

	if (!IsLocalController())
	{
		return;
	}

	CreateRankingBoard();
}

void APTWPlayerController::BindAmmoDelegate()
{
	if (!CurrentWeaponItem)
	{
		return;
	}

	CurrentWeaponItem->OnAmmoChanged.AddUObject(this, &ThisClass::HandleAmmoChanged);
}

void APTWPlayerController::UnbindAmmoDelegate()
{
	if (CurrentWeaponItem)
	{
		CurrentWeaponItem->OnAmmoChanged.RemoveAll(this);
	}
}

void APTWPlayerController::HandleAmmoChanged(int32 CurrentAmmo, int32 MaxAmmo)
{
	if (!IsLocalController())
	{
		return;
	}

	if (APTWHUD* PTWHUD = Cast<APTWHUD>(GetHUD()))
	{
		PTWHUD->UpdateAmmo(CurrentAmmo, MaxAmmo);
	}
}

void APTWPlayerController::SyncAmmoUIOnce()
{
	if (!CurrentWeaponItem)
		return;

	HandleAmmoChanged(CurrentWeaponItem->CurrentAmmo, CurrentWeaponItem->GetMaxAmmo());
}

void APTWPlayerController::HandleWeaponChanged(FGameplayTag NewWeaponTag, UPTWItemInstance* NewItemInstance)
{
	if (!IsLocalController() || !NewItemInstance)
	{
		return;
	}

	// 1. 이전 무기 델리게이트 해제
	UnbindAmmoDelegate();

	// 2. 교체
	CurrentWeaponItem = NewItemInstance;

	// 3. 새 무기 델리게이트 바인딩
	BindAmmoDelegate();

	// 4. 초기 UI 동기화
	SyncAmmoUIOnce();
}
