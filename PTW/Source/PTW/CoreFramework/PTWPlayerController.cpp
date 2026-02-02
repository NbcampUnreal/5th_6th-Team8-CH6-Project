// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWPlayerController.h"
#include "PTWPlayerState.h"

#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/GameState.h"
#include "GameplayTagContainer.h"
#include "EngineUtils.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Pawn.h"

#include "CoreFramework/PTWBaseCharacter.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "UI/PTWUISubsystem.h"
#include "UI/PTWHUD.h"
#include "UI/PTWInGameHUD.h"
#include "UI/RankBoard/PTWRankingBoard.h"
#include "Inventory/PTWItemInstance.h"
#include "Inventory/PTWWeaponActor.h"

void APTWPlayerController::StartSpectating()
{
	if (HasAuthority())
	{
		MulticastRPC_StartSpectating();
	}
}

void APTWPlayerController::MulticastRPC_StartSpectating_Implementation()
{
	if (IsLocalController())
	{
		if (!OnPossessedPawnChanged.IsAlreadyBound(this, &ThisClass::SpectateNextPlayer))
		{
			OnPossessedPawnChanged.AddDynamic(this, &ThisClass::SpectateNextPlayer);
		}
	}

	UnPossess();
	ChangeState(NAME_Spectating);
	if (HasAuthority())
	{
		ClientGotoState(NAME_Spectating);
	}
}

void APTWPlayerController::SpectateNextPlayer(APawn* InOldPawn, APawn* InNewPawn)
{
	if (OnPossessedPawnChanged.IsAlreadyBound(this, &ThisClass::SpectateNextPlayer))
	{
		OnPossessedPawnChanged.RemoveDynamic(this, &ThisClass::SpectateNextPlayer);
	}

	if (IsValid(InNewPawn)) return;

	if (APawn* NewTargetView = FindNextSpectatorTarget(InNewPawn))
	{
		SetSpectatorTarget(NewTargetView);
	}
}

APawn* APTWPlayerController::FindNextSpectatorTarget(APawn* InNewPawn)
{
	if (IsValid(InNewPawn))
	{
		return nullptr;
	}

	if (!IsValid(PlayerState))
	{
		return nullptr;
	}

	if (PlayerState->GetPawn() || GetPawn())
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return nullptr;
	}

	AGameStateBase* GS = World->GetGameState();
	if (!IsValid(GS))
	{
		return nullptr;
	}

	const TArray<APlayerState*>& PlayArray = GS->PlayerArray;
	if (PlayArray.IsEmpty())
	{
		return nullptr;
	}

	APawn* CurrentViewTarget = nullptr;
	APawn* NewViewTarget = nullptr;

	if (AActor* CurrentViewTargetActor = GetViewTarget())
	{
		CurrentViewTarget = Cast<APawn>(CurrentViewTargetActor);
		if (IsValid(CurrentViewTarget))
		{
			if (APlayerState* CurrentViewTargetPS = CurrentViewTarget->GetPlayerState())
			{
				if (!IsValid(CurrentViewTargetPS))
				{
					CurrentViewTargetPS = PlayerState;
				}

				int32 FoundIndex = PlayArray.Find(CurrentViewTargetPS);
				if (FoundIndex != INDEX_NONE)
				{
					for (int32 i = FoundIndex + 1; i < PlayArray.Num(); i++)
					{
						if (PlayArray[i] && PlayArray[i]->GetPawn() && !PlayArray[i]->IsSpectator())
						{
							NewViewTarget = PlayArray[i]->GetPawn();
							break;
						}
					}
				}
			}
		}
	}

	if (!IsValid(NewViewTarget))
	{
		for (APlayerState* PS : PlayArray)
		{
			if (PS && (PS != PlayerState) && (!PS->IsSpectator()) && PS->GetPawn())
			{
				NewViewTarget = PS->GetPawn();
				break;
			}
		}
	}

	if (IsValid(NewViewTarget))
	{
		if (!IsValid(CurrentViewTarget) || (CurrentViewTarget != NewViewTarget))
		{
			return NewViewTarget;
		}
	}
	return nullptr;
}

void APTWPlayerController::SetSpectatorTarget(APawn* NewViewTarget)
{
	TWeakObjectPtr<ThisClass> WeakThis = this;
	TWeakObjectPtr<APawn> WeakViewTarget = NewViewTarget;
	FTimerHandle NextViewTimerHandle;
	GetWorldTimerManager().SetTimer(NextViewTimerHandle, [WeakThis, WeakViewTarget]()
		{
			if (WeakThis.IsValid() && WeakViewTarget.IsValid() && !IsValid(WeakThis->GetPawn()))
			{
				WeakThis->SetViewTargetWithBlend(WeakViewTarget.Get(), 0.5f, VTBlend_Cubic);
			}
		}, 0.1f, false);
}

void APTWPlayerController::OnInputSpectateNext()
{
	if (GetStateName() == NAME_Spectating)
	{
		SpectateNextPlayer(GetPawn(), GetPawn());
	}
}

void APTWPlayerController::ClientRPC_ShowDamageIndicator_Implementation(FVector DamageCauserLocation)
{
	if (IsLocalController())
	{
		UISubsystem->ShowDamageIndicator(DamageCauserLocation);
	}
}

void APTWPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}

	//EquipTag = FGameplayTag::RequestGameplayTag(TEXT("Weapon.State.Equip"));
	//SprintTag = FGameplayTag::RequestGameplayTag(TEXT("State.Movement.Sprinting"));

	/* 플레이어 이름 가시성 체크 타이머 */
	GetWorldTimerManager().SetTimer(NameTagTimerHandle, this, &APTWPlayerController::UpdateNameTagsVisibility, NameTagUpdateInterval, true);

	/* UI 서브시스템 등록 */
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		UISubsystem = LP->GetSubsystem<UPTWUISubsystem>();
	}

	/* Input Mapping Context 추가 */
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	/* UI 위젯 생성 */
	CreateUI();
}

void APTWPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		GetWorldTimerManager().ClearTimer(RespawnTimerHandle);
	}
	
	Super::EndPlay(EndPlayReason);
}

void APTWPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (!IsLocalController())
	{
		return;
	}

	CreateUI();
}

void APTWPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();
}

void APTWPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void APTWPlayerController::OnUnPossess()
{
	Super::OnUnPossess();
}

void APTWPlayerController::BeginSpectatingState()
{
	Super::BeginSpectatingState();
}

void APTWPlayerController::SetViewTarget(AActor* NewViewTarget, FViewTargetTransitionParams TransitionParams)
{
	AActor* PrevViewTarget = GetViewTarget();
	Super::SetViewTarget(NewViewTarget, TransitionParams);
	
	if (APTWPlayerCharacter* PlayerCharacter = Cast<APTWPlayerCharacter>(PrevViewTarget))
	{
		SetSetOnlyOwnerSeeRecursive(PlayerCharacter->GetMesh1P(), true);
		SetOwnerNoSeeRecursive(PlayerCharacter->GetMesh1P(), true);
			
		SetSetOnlyOwnerSeeRecursive(PlayerCharacter->GetMesh3P(), false);
		SetOwnerNoSeeRecursive(PlayerCharacter->GetMesh3P(), false);
	}
	
	if (APTWPlayerCharacter* PlayerCharacter = Cast<APTWPlayerCharacter>(NewViewTarget))
	{
		if (GetPawn() == NewViewTarget)		// 내 카메라를 사용할 경우
		{
			SetSetOnlyOwnerSeeRecursive(PlayerCharacter->GetMesh1P(), true);
			SetOwnerNoSeeRecursive(PlayerCharacter->GetMesh1P(), false);
			
			SetSetOnlyOwnerSeeRecursive(PlayerCharacter->GetMesh3P(), false);
			SetOwnerNoSeeRecursive(PlayerCharacter->GetMesh3P(), true);
		}
		else
		{
			SetSetOnlyOwnerSeeRecursive(PlayerCharacter->GetMesh1P(), false);
			SetOwnerNoSeeRecursive(PlayerCharacter->GetMesh1P(), false);
			
			SetSetOnlyOwnerSeeRecursive(PlayerCharacter->GetMesh3P(), true);
			SetOwnerNoSeeRecursive(PlayerCharacter->GetMesh3P(), true);
		}
	}
}

void APTWPlayerController::SetOwnerNoSeeRecursive(USceneComponent* InParentComponent, bool bNewOwnerNoSee)
{
	if (!InParentComponent) return;
	
	if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(InParentComponent))
	{
		PrimComp->SetOwnerNoSee(bNewOwnerNoSee);
	}
	
	TArray<USceneComponent*> _Children = InParentComponent->GetAttachChildren();
	for (USceneComponent* Child : _Children)
	{
		SetOwnerNoSeeRecursive(Child, bNewOwnerNoSee);
	}
}

void APTWPlayerController::SetSetOnlyOwnerSeeRecursive(USceneComponent* InParentComponent, bool bNewOnlyOwnerSee)
{
	if (!InParentComponent) return;
	
	if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(InParentComponent))
	{
		PrimComp->SetOnlyOwnerSee(bNewOnlyOwnerSee);
	}
	
	TArray<USceneComponent*> _Children = InParentComponent->GetAttachChildren();
	for (USceneComponent* Child : _Children)
	{
		SetSetOnlyOwnerSeeRecursive(Child, bNewOnlyOwnerSee);
	}
}

//void APTWPlayerController::BindASCDelegates()
//{
//	
//}
//
//void APTWPlayerController::UnbindASCDelegates()
//{
//	
//}

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

		// ESC / Pause Menu
		EIC->BindAction(
			PauseMenuAction,
			ETriggerEvent::Started,
			this,
			&APTWPlayerController::HandleMenuInput
		);
	}
}

void APTWPlayerController::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();

	// 로컬 컨트롤러인지 다시 확인
	if (IsLocalController())
	{
		// 서브시스템 캐싱 (레벨 이동 후 PC가 새로 생성될 수 있으므로 다시 할당)
		if (ULocalPlayer* LP = GetLocalPlayer())
		{
			UISubsystem = LP->GetSubsystem<UPTWUISubsystem>();
		}

		// 위젯 재생성 및 뷰포트 재등록
		CreateUI();
	}
}

void APTWPlayerController::CreateUI()
{
	if (UISubsystem)
	{
		UISubsystem->ShowHUD(HUDClass);
		UISubsystem->CreatePersistentWidget(RankingBoardClass, 10);
	}
}

void APTWPlayerController::OnRankingPressed()
{
	if(UISubsystem)
	{
		UISubsystem->SetWidgetVisibility(RankingBoardClass, true);
	}
}

void APTWPlayerController::OnRankingReleased()
{
	if (UISubsystem)
	{
		UISubsystem->SetWidgetVisibility(RankingBoardClass, false);
	}
}

void APTWPlayerController::HandleMenuInput()
{
	if (!IsLocalController()) return;

	if (!UISubsystem->IsStackEmpty())
	{
		// UI 스택의 최상단 위젯을 닫음 
		UISubsystem->PopWidget();
	}
	else
	{
		// 아무런 위젯이 없을 때는 PauseMenu 띄우기
		if (PauseMenuClass)
		{
			UISubsystem->PushWidget(PauseMenuClass, EUIInputPolicy::GameAndUI);
		}
	}
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

void APTWPlayerController::Client_SetInputRestricted_Implementation(bool bRestricted)
{
	SetIgnoreMoveInput(bRestricted);
	SetIgnoreLookInput(bRestricted);
}
