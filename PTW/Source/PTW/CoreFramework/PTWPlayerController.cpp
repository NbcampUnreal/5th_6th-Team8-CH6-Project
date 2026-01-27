// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWPlayerController.h"
#include "PTWPlayerState.h"

#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameplayTagContainer.h"
#include "EngineUtils.h"
#include "Components/WidgetComponent.h"

#include "CoreFramework/PTWBaseCharacter.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "UI/PTWHUD.h"
#include "UI/RankBoard/PTWRankingBoard.h"
#include "Inventory/PTWItemInstance.h"
#include "Inventory/PTWWeaponActor.h"

void APTWPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}

	/* HUD 초기화 */
	UE_LOG(LogTemp, Error, TEXT("Controller BeginPlay"));
	TryInitializeHUD();

	/* 플레이어 이름 가시성 체크 타이머 */
	GetWorldTimerManager().SetTimer(NameTagTimerHandle, this, &APTWPlayerController::UpdateNameTagsVisibility, NameTagUpdateInterval, true);

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

void APTWPlayerController::UpdateNameTagsVisibility()
{
	APawn* MyPawn = GetPawn();
	if (!MyPawn || !PlayerCameraManager) return;

	const FVector CameraLocation = PlayerCameraManager->GetCameraLocation();
	const FVector CameraForward = PlayerCameraManager->GetActorForwardVector();
	const float   MaxDistSq = FMath::Square(NameTagMaxDistance);

	for (TActorIterator<APTWPlayerCharacter> It(GetWorld()); It; ++It)
	{
		APTWPlayerCharacter* TargetChar = *It;
		if (!TargetChar) continue;

		// 자기 자신 / 사망 체크
		if (TargetChar == MyPawn || TargetChar->IsDead())
		{
			if (UWidgetComponent* WidgetComp = TargetChar->GetNameTagWidget())
			{
				WidgetComp->SetVisibility(false);
			}
			continue;
		}

		UWidgetComponent* WidgetComp = TargetChar->GetNameTagWidget();
		if (!WidgetComp) continue;

		// 거리 체크 (DistSquared)
		const FVector TargetLocation = TargetChar->GetActorLocation();
		const float DistSq = FVector::DistSquared(CameraLocation, TargetLocation);

		if (DistSq > MaxDistSq)
		{
			WidgetComp->SetVisibility(false);
			continue;
		}

		// 시야각(FOV) 체크
		const FVector ToTarget = (TargetLocation - CameraLocation).GetSafeNormal();
		const float Dot = FVector::DotProduct(CameraForward, ToTarget);

		if (Dot < 0.3f) // 약 72도
		{
			WidgetComp->SetVisibility(false);
			continue;
		}

		// 벽 가림 체크 (Line Trace)
		const FVector TraceEnd = WidgetComp->GetComponentLocation();

		FHitResult HitResult;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(NameTagTrace), false);
		Params.AddIgnoredActor(MyPawn);
		Params.AddIgnoredActor(TargetChar);

		const bool bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			CameraLocation,
			TraceEnd,
			ECC_Visibility,
			Params
		);

		// 가시성 설정 및 스케일 조절 추가
		if (!bHit)
		{
			WidgetComp->SetVisibility(true);

			// 거리에 따른 스케일 계산 
			const float CurrentDist = FMath::Sqrt(DistSq);

			// 거리 0(스케일 1.0) ~ MaxDist(스케일 MinScale) 사이를 매핑
			float TargetScale = FMath::GetMappedRangeValueClamped(
				FVector2D(0.f, NameTagMaxDistance),
				FVector2D(1.0f, NameTagMinScale),
				CurrentDist
			);

			// 위젯의 RenderScale을 조절 (해상도 저하 없이 크기만 조절)
			if (UUserWidget* UserW = WidgetComp->GetUserWidgetObject())
			{
				UserW->SetRenderScale(FVector2D(TargetScale, TargetScale));
			}
		}
		else
		{
			WidgetComp->SetVisibility(false);
		}
	}
}
