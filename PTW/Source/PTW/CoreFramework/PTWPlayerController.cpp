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
#include "GameFramework/SpectatorPawn.h"
#include "UI/PTWUISubsystem.h"
#include "UI/PTWHUD.h"
#include "UI/PTWInGameHUD.h"
#include "UI/RankBoard/PTWRankingBoard.h"
#include "UI/ChatWidget/PTWChatList.h"
#include "UI/ChatWidget/PTWChatInput.h"
#include "UI/InGameUI/PTWDamageIndicator.h"
#include "UI/MiniGame/PTWGameStartTimer.h"
#include "Inventory/Instance/PTWItemInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "System/PTWSessionSubsystem.h"
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
	if (HasAuthority())
	{
		UnPossess();
		ChangeState(NAME_Spectating);
		ClientGotoState(NAME_Spectating);
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

void APTWPlayerController::Client_PrepareLoadingScreen_Implementation(ELoadingScreenType Type, FName MapRowName)
{
	if (UPTWGameInstance* GI = GetGameInstance<UPTWGameInstance>())
	{
		GI->PrepareLoadingScreen(Type, MapRowName);
	}
}

void APTWPlayerController::Client_DisplayLoadingScreen_Implementation()
{
	if (UPTWGameInstance* GI = GetGameInstance<UPTWGameInstance>())
	{
		GI->DisplayLoadingScreen();
	}
}

void APTWPlayerController::Client_OpenMainMenu_Implementation()
{
	UPTWGameInstance* GI = GetGameInstance<UPTWGameInstance>();
	if (!GI) return;
	
	if (UPTWSessionSubsystem* SessionSubsystem = GI->GetSubsystem<UPTWSessionSubsystem>())
	{
		SessionSubsystem->LeaveGameSession();
	}
}

void APTWPlayerController::Server_NotifyReadyToPlay_Implementation()
{
	if (APTWGameMode* GameMode = GetWorld()->GetAuthGameMode<APTWGameMode>())
	{
		GameMode->PlayerReadyToPlay(this);
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

ASpectatorPawn* APTWPlayerController::SpawnSpectatorPawn()
{
	ASpectatorPawn* SpawnedSpectator = nullptr;

	// Only spawned for the local player
	if ((GetSpectatorPawn() == nullptr) && IsLocalController())
	{
		UWorld* World = GetWorld();
		if (AGameStateBase const* const GameState = World->GetGameState())
		{
			if (UClass* SpectatorClass = GameState->SpectatorClass)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = this;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				SpawnParams.ObjectFlags |= RF_Transient;	// We never want to save spectator pawns into a map
				
				FVector DeadActorLocation = GetSpawnLocation();
				FRotator DeadActorRotation = GetControlRotation();
				
				if (APawn* DeadPawn = GetPawn())
				{
					DeadActorLocation = DeadPawn->GetActorLocation();
					DeadActorRotation = DeadPawn->GetActorRotation();
				}
				
				SpawnedSpectator = World->SpawnActor<ASpectatorPawn>(SpectatorClass, DeadActorLocation, DeadActorRotation, SpawnParams);
				if (SpawnedSpectator)
				{
					SpawnedSpectator->SetReplicates(false); // Client-side only
					SpawnedSpectator->PossessedBy(this);
					SpawnedSpectator->DispatchRestart(true);
					if (SpawnedSpectator->PrimaryActorTick.bStartWithTickEnabled)
					{
						SpawnedSpectator->SetActorTickEnabled(true);
					}

					UE_LOG(LogPlayerController, Verbose, TEXT("Spawned spectator %s [server:%d]"), *GetNameSafe(SpawnedSpectator), GetNetMode() < NM_Client);
				}
				else
				{
					UE_LOG(LogPlayerController, Warning, TEXT("Failed to spawn spectator with class %s"), *GetNameSafe(SpectatorClass));
				}
			}
		}
		else
		{
			// This normally happens on clients if the Player is replicated but the GameState has not yet.
			UE_LOG(LogPlayerController, Verbose, TEXT("NULL GameState when trying to spawn spectator!"));
		}
	}

	return SpawnedSpectator;
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

		// 키가이드 (K)
		EIC->BindAction(
			KeyGuideAction,
			ETriggerEvent::Started,
			this,
			&APTWPlayerController::OnKeyGuidePressed
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

		// 서버에 SeamlessTravel을 했다고 알림
		Server_NotifyReadyToPlay();
	}
}

void APTWPlayerController::CreateUI()
{
	if (UISubsystem)
	{
		UISubsystem->StackReset();
		if (HUDClass)
		{
			UISubsystem->ShowHUD(HUDClass);
		}
		if (RankingBoardClass)
		{
			UISubsystem->CreatePersistentWidget(RankingBoardClass, 10);
			bAbleRankingBoard = true;
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
		if (KeyGuideWidgetClass)
		{
			UISubsystem->CreatePersistentWidget(KeyGuideWidgetClass, 15);
			UISubsystem->SetWidgetVisibility(KeyGuideWidgetClass, true);
			bKeyGuideOn = true;
		}
	}
}

void APTWPlayerController::OnRankingPressed()
{
	if(UISubsystem && bAbleRankingBoard)
	{
		UISubsystem->SetWidgetVisibility(RankingBoardClass, true);
	}
}

void APTWPlayerController::OnRankingReleased()
{
	if (UISubsystem && bAbleRankingBoard)
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

void APTWPlayerController::OnKeyGuidePressed()
{
	if (!KeyGuideWidgetClass) return;

	bKeyGuideOn = !bKeyGuideOn;

	if (UISubsystem)
	{
		UISubsystem->SetWidgetVisibility(KeyGuideWidgetClass, bKeyGuideOn);
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
		if (TargetChar == MyPawn || TargetChar->IsDead() || TargetChar->GetStealthMode())
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

void APTWPlayerController::Client_SetInputRestricted_Implementation(bool bRestricted)
{
	SetIgnoreMoveInput(bRestricted);
	SetIgnoreLookInput(bRestricted);
}

void APTWPlayerController::ApplyInputRestricted(bool bRestricted)
{
	if (!IsLocalController()) return;
	
	SetIgnoreMoveInput(bRestricted);
	SetIgnoreLookInput(bRestricted);
	
	APTWPlayerCharacter* PC = Cast<APTWPlayerCharacter>(GetPawn());
	if (!PC) return;

	UInputMappingContext* IMC = PC->GetDefaultMappingContext();
	if (!IMC) return;

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer) return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
	if (!Subsystem) return;

	if (bRestricted)
		Subsystem->RemoveMappingContext(IMC);
	else
		Subsystem->AddMappingContext(IMC, 0);
}
