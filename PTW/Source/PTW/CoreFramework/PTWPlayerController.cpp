// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWPlayerController.h"
#include "PTWPlayerState.h"

#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/GameState.h"
#include "GameFramework/PlayerState.h"
#include "GameplayTagContainer.h"
#include "EngineUtils.h"
#include "Components/WidgetComponent.h"

#include "CoreFramework/PTWGameUserSettings.h"
#include "CoreFramework/PTWBaseCharacter.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "CoreFramework/Game/GameMode/PTWGameMode.h"
#include "CoreFramework/Game/GameInstance/PTWGameInstance.h"
#include "UI/PTWUISubsystem.h"
#include "UI/PTWHUD.h"
#include "UI/PTWInGameHUD.h"
#include "UI/RankBoard/PTWRankingBoard.h"
#include "UI/ChatWidget/PTWChatList.h"
#include "UI/ChatWidget/PTWChatInput.h"
#include "UI/InGameUI/PTWDamageIndicator.h"
#include "UI/MiniGame/PTWGameStartTimer.h"
#include "Inventory/Instance/PTWItemInstance.h"
#include "Weapon/PTWWeaponActor.h"

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

bool APTWPlayerController::Server_SendChatMessage_Validate(const FString& Message)
{
	// 너무 긴 메시지를 보내 서버를 공격하는 것을 방지 (200자 제한)
	return Message.Len() <= 200;
}

void APTWPlayerController::Server_SendChatMessage_Implementation(const FString& Message)
{
	FString SenderName = TEXT("Unknown");

	if (APTWPlayerState* PS = GetPlayerState<APTWPlayerState>())
	{
		FPTWPlayerData Data = PS->GetPlayerData();

		if (!Data.PlayerName.IsEmpty()) SenderName = Data.PlayerName;
		else SenderName = PS->GetPlayerName();
	}

	if (APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>())
	{
		GS->BroadcastChatMessage(SenderName, Message);
	}
}

void APTWPlayerController::OnChatInputFinished()
{
	if (UISubsystem)
	{
		/* 리스트 위젯을 다시 투명 모드로 되돌림 */
		if (UPTWChatList* ChatList = Cast<UPTWChatList>(UISubsystem->GetOrCreateWidget(ChatListClass)))
		{
			ChatList->SetInteractionMode(false);
		}
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

	BindGameStateDelegates();

	/* UI 위젯 생성 */
	CreateUI();

	/* 게임설정 */
	if (UPTWGameUserSettings* Settings = Cast<UPTWGameUserSettings>(UGameUserSettings::GetGameUserSettings()))
	{
		CurrentMouseSensitivity = Settings->MouseSensitivity;
	}
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

	BindGameStateDelegates();

	CreateUI();
}

void APTWPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();
}

void APTWPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	/* 로딩완료 */
	Server_ReportLoadingComplete();
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

void APTWPlayerController::BindGameStateDelegates()
{
	APTWGameState* GS = GetWorld() ? GetWorld()->GetGameState<APTWGameState>() : nullptr;
	if (!GS) return;

	// 중복 바인딩 방지
	UnbindGameStateDelegates();

	// 델리게이트 바인드
	GS->OnMiniGameCountdownChanged.AddDynamic(this, &ThisClass::OnMiniGameCountdownChanged);
	GS->OnRoulettePhaseChanged.AddDynamic(this, &ThisClass::HandleRoulettePhaseChanged);

	// 현재상태 반영
	OnMiniGameCountdownChanged(GS->IsMiniGameCountdown());
	HandleRoulettePhaseChanged(GS->GetRouletteData());
}

void APTWPlayerController::UnbindGameStateDelegates()
{
	if (APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>())
	{
		GS->OnMiniGameCountdownChanged.RemoveAll(this);
		GS->OnRoulettePhaseChanged.RemoveAll(this);
	}
}

void APTWPlayerController::OnMiniGameCountdownChanged(bool bStarted)
{
	if (!UISubsystem || !GameStartTimerClass)
		return;

	if (bStarted)
	{
		UISubsystem->ShowSystemWidget(GameStartTimerClass, 70);
	}
	else
	{
		UISubsystem->HideSystemWidget(GameStartTimerClass);
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

void APTWPlayerController::HandleRoulettePhaseChanged(FPTWRouletteData RouletteData)
{
	const UEnum* EnumPtr = StaticEnum<EPTWRoulettePhase>();
	FString PhaseName = EnumPtr ? EnumPtr->GetNameStringByValue((int64)RouletteData.CurrentPhase) : TEXT("Invalid");

	UE_LOG(LogTemp, Error, TEXT("PTWPlayerController : HandleRoulettePhaseChanged - %s"), *PhaseName);

	if (!UISubsystem)
	{
		return;
	}

	switch (RouletteData.CurrentPhase)
	{
	case EPTWRoulettePhase::MapRoulette:
		if (MapRouletteWidgetClass)
		{
			UISubsystem->ShowSystemWidget(MapRouletteWidgetClass, 85);
		}
		break;
	case EPTWRoulettePhase::RoundEventRoulette:
		if (MapRouletteWidgetClass)
		{
			UISubsystem->HideSystemWidget(MapRouletteWidgetClass);
		}
		break;
	case EPTWRoulettePhase::Finished:
		if (MapRouletteWidgetClass)
		{
			UISubsystem->HideSystemWidget(MapRouletteWidgetClass);
		}
		break;

	default:
		break;
	}
}

void APTWPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// 랭킹보드 (Tab)
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

		// Pause Menu (ESC)
		EIC->BindAction(
			PauseMenuAction,
			ETriggerEvent::Started,
			this,
			&APTWPlayerController::HandleMenuInput
		);

		// 채팅 (Enter)
		EIC->BindAction(
			ChattingAction,
			ETriggerEvent::Started,
			this,
			&APTWPlayerController::OnChatPressed
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
		UE_LOG(LogTemp, Error, TEXT("PTWPlayerController : PostSeamlessTravel"));
		CreateUI();
	}
}

void APTWPlayerController::CreateUI()
{
	if (UISubsystem)
	{
		if (HUDClass)
		{
			UISubsystem->ShowHUD(HUDClass);
		}
		if (RankingBoardClass)
		{
			UISubsystem->CreatePersistentWidget(RankingBoardClass, 10);
		}
		if (ChatListClass)
		{
			if (UUserWidget* ChatListWidget = UISubsystem->CreatePersistentWidget(ChatListClass, 70))
			{
				ChatListWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			}
			UISubsystem->SetChatListClass(ChatListClass);
		}
		if (UISubsystem)
		{
			UISubsystem->SetChatInputClass(ChatInputClass);
		}
		if (DamageIndicatorClass)
		{
			UISubsystem->SetDamageIndicatorClass(DamageIndicatorClass);
		}
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

void APTWPlayerController::OnChatPressed()
{
	if (!UISubsystem || !ChatInputClass || !ChatListClass) return;

	/* 이미 채팅 입력창이 떠 있는지 확인 */
	if (UISubsystem->IsWidgetInStack(ChatInputClass))
	{
		return;
	}
	else
	{
		/* 채팅창 새로 열기(Push) */
		UISubsystem->PushWidget(ChatInputClass, EUIInputPolicy::GameAndUI);

		/* ChatList를 찾아 활성화 */
		if (UPTWChatList* ChatList = Cast<UPTWChatList>(UISubsystem->GetOrCreateWidget(ChatListClass)))
		{
			ChatList->SetInteractionMode(true);
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

void APTWPlayerController::Server_ReportLoadingComplete_Implementation()
{
	if (APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>())
	{
		GS->LoadedPlayerCount++;
	}

	if (APTWGameMode* GM = GetWorld()->GetAuthGameMode<APTWGameMode>())
	{
		GM->CheckAllPlayersLoaded();
	}
}

void APTWPlayerController::ApplyMouseSensitivity(float NewValue)
{
	CurrentMouseSensitivity = NewValue;
}

void APTWPlayerController::Client_PrepareLoadingScreen_Implementation(ELoadingScreenType Type, FName MapRowName)
{
	if (UPTWGameInstance* GI = GetGameInstance<UPTWGameInstance>())
	{
		GI->PrepareLoadingScreen(Type, MapRowName);
	}
}

void APTWPlayerController::Client_SetInputRestricted_Implementation(bool bRestricted)
{
	SetIgnoreMoveInput(bRestricted);
	SetIgnoreLookInput(bRestricted);
}
