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
#include "Engine/TextureRenderTarget2D.h"
#include "Components/SceneCaptureComponent2D.h" 
#include "TimerManager.h"

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
#include "UI/MiniGame/Bomb/PTWBombWarning.h"
#include "UI/MiniGame/GhostChase/PTWTargetViewWidget.h"
#include "Inventory/Instance/PTWItemInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "System/PTWSessionSubsystem.h"
#include "Weapon/PTWWeaponActor.h"
#include "MiniGame/Item/BombItem/PTWBombActor.h"
#include "OnlineSubsystemUtils.h"
#include "UI/Dev/PTWDevWidget.h"
#include "CoreFramework/Character/Component/PTWDeveloperComponent.h"
#include "Engine/PostProcessVolume.h"
#include "EngineUtils.h"
#include "Game/GameMode/PTWLobbyGameMode.h"
#include "PTWGameplayTag/GameplayTags.h"

APTWPlayerController::APTWPlayerController()
{
	DeveloperComponent = CreateDefaultSubobject<UPTWDeveloperComponent>(TEXT("DevComponent"));
}

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

		UISubsystem->PopWidget();
	}

	bAbleChat = true;
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

void APTWPlayerController::BindBombDelegate(APTWBombActor* NewBomb)
{
	// 중복 바인드 방지
	UnBindBombDelegate();

	CachedBombActor = NewBomb;

	if (CachedBombActor)
	{
		CachedBombActor->OnBombOwnerChanged.AddUObject(this, &APTWPlayerController::HandleBombOwnerChanged);
	}

	// UI 생성
	if (!BombWarningWidgetClass) return;

	UUserWidget* Widget = UISubsystem->ShowSystemWidget(BombWarningWidgetClass, 70);
	if (UPTWBombWarning* BombWidget = Cast<UPTWBombWarning>(Widget))
	{
		BombWidget->SetTargetBomb(CachedBombActor);
	}
	UISubsystem->SetWidgetVisibility(BombWarningWidgetClass, false);

	// 바인딩 시점에 이미 폭탄 주인이 결정되어 있다면 UI에 전달
	if (CachedBombActor->GetBombOwnerPawn())
	{
		HandleBombOwnerChanged(CachedBombActor->GetBombOwnerPawn());
	}
}

void APTWPlayerController::UnBindBombDelegate()
{
	// 델리게이트 제거
	if (CachedBombActor)
	{
		CachedBombActor->OnBombOwnerChanged.RemoveAll(this);
	}

	// UI 제거
	if (!BombWarningWidgetClass) return;

	UISubsystem->HideSystemWidget(BombWarningWidgetClass);
}

void APTWPlayerController::OnVoicePressed()
{
	if (IsLocalPlayerController())
	{
		IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
		if (Subsystem)
		{
			IOnlineVoicePtr VoiceInterface = Subsystem->GetVoiceInterface();
			if (VoiceInterface.IsValid())
			{
				UE_LOG(LogTemp, Warning, TEXT("OnVoicePressed"))
				VoiceInterface->StartNetworkedVoice(0);
			}
		}
	}
}

void APTWPlayerController::OnVoiceReleased()
{
	if (IsLocalPlayerController())
	{
		IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
		if (Subsystem)
		{
			IOnlineVoicePtr VoiceInterface = Subsystem->GetVoiceInterface();
			if (VoiceInterface.IsValid())
			{
				VoiceInterface->StopNetworkedVoice(0);
			}
		}
	}
}

void APTWPlayerController::Client_ShowNotification_Implementation(const FNotificationData& Data)
{
	if (!IsLocalController()) return;

	if (UISubsystem)
	{
		UISubsystem->PushNotification(Data);
	}
}

void APTWPlayerController::ShowLocalNotification(const FNotificationData& Data)
{
	if (!IsLocalController()) return;

	if (UISubsystem)
	{
		UISubsystem->PushNotification(Data);
	}
}

void APTWPlayerController::SendMessage(const FText& InText, ENotificationPriority InPriority, float InDuration, bool bInterrupt)
{
	if (!HasAuthority()) return; // 서버에서만 호출

	FNotificationData Data;
	Data.Message = InText;
	Data.Priority = InPriority;
	Data.Duration = InDuration;
	Data.bInterrupt = bInterrupt;

	Client_ShowNotification(Data);
}

void APTWPlayerController::UpdateTargetPOV(APawn* NewTarget)
{
	// 초기화: 새로운 타겟을 설정하기 전에 기존에 돌아가던 타이머 제거
	GetWorldTimerManager().ClearTimer(POVCaptureTimerHandle);

	// 기존에 연결된 캡처 컴포넌트가 있다면 연결을 끊고 참조를 초기화
	if (CurrentActiveCapture)
	{
		/*CurrentActiveCapture->TextureTarget = nullptr;
		CurrentActiveCapture = nullptr;*/
		RefreshTargetViewHiddenActors();
	}

	if (!NewTarget || !IsLocalController()) return;

	// RenderTarget 리소스 최적화 생성
	if (!TargetPOVRT)
	{
		TargetPOVRT = NewObject<UTextureRenderTarget2D>(this);
		// PF_B8G8R8A8: HDR이 필요 없는 UI용 화면에 가장 적합한 8비트 포맷 (메모리 절약)
		TargetPOVRT->InitCustomFormat(480, 270, PF_B8G8R8A8, false);
		// 리소스가 즉시 GPU 메모리에 할당되도록 강제 업데이트
		TargetPOVRT->UpdateResourceImmediate(true);
	}

	// 타겟 캐릭터 및 캡처 컴포넌트 확보
	if (APTWPlayerCharacter* TargetChar = Cast<APTWPlayerCharacter>(NewTarget))
	{
		// 타겟 캐릭터 내부에 미리 생성해둔 전용 캡처 컴포넌트를 가져옴 (Getter 활용)
		CurrentActiveCapture = TargetChar->GetTargetPOVCapture();
		if (CurrentActiveCapture)
		{
			// 캡처 결과물을 위에서 만든 RenderTarget으로 출력하도록 설정
			CurrentActiveCapture->TextureTarget = TargetPOVRT;

			// 캡처 화면에서 타겟 자신을 숨김 (도망자의 1인칭 시점 효과)
			CurrentActiveCapture->HiddenActors.Empty();
			CurrentActiveCapture->HiddenActors.Add(TargetChar);

			// 타겟 캐릭터에 부착된 모든 액터(무기, 액세서리 등)도 함께 숨김 처리
			TArray<AActor*> AttachedActors;
			TargetChar->GetAttachedActors(AttachedActors);
			CurrentActiveCapture->HiddenActors.Append(AttachedActors);

			// 성능 최적화: 수동 캡처 타이머 시작
			// 0.033f = 약 30 FPS. 매 프레임(bCaptureEveryFrame) 대신 타이머를 써서 GPU 부하를 1/3로 감소
			GetWorldTimerManager().SetTimer(
				POVCaptureTimerHandle,
				this,
				&APTWPlayerController::CaptureTargetPOV,
				0.033f,
				true
			);

			if (UISubsystem && POVWidgetClass)
			{
				// 시스템 위젯으로 생성
				UISubsystem->ShowSystemWidget(POVWidgetClass);

				// 생성된 위젯을 가져와서 렌더 타겟 연결
				if (UPTWTargetViewWidget* POVWidget = Cast<UPTWTargetViewWidget>(UISubsystem->GetOrCreateWidget(POVWidgetClass)))
				{
					POVWidget->SetRenderTarget(TargetPOVRT);
					// 확실하게 보이도록 설정
					UISubsystem->SetWidgetVisibility(POVWidgetClass, true);
				}
			}
		}
	}
}

void APTWPlayerController::RefreshTargetViewHiddenActors()
{
	if (!CurrentActiveCapture) return;

	// 목록 초기화
	CurrentActiveCapture->HiddenActors.Empty();

	// 타겟(추적 대상)과 그 부착물들 숨기기 (원래 로직)
	if (APawn* TargetPawn = Cast<APawn>(CurrentActiveCapture->GetOwner()))
	{
		CurrentActiveCapture->HiddenActors.Add(TargetPawn);
		TArray<AActor*> TargetAttached;
		TargetPawn->GetAttachedActors(TargetAttached);
		CurrentActiveCapture->HiddenActors.Append(TargetAttached);
	}

	// 플레이어가 유령 상태면 숨기기
	APTWPlayerState* PS = GetPlayerState<APTWPlayerState>();
	APTWPlayerCharacter* MyChar = Cast<APTWPlayerCharacter>(GetPawn());

	if (PS && MyChar)
	{
		UAbilitySystemComponent* MyASC = PS->GetAbilitySystemComponent();
		if (MyASC)
		{
			bool bIsInvisible = MyASC->HasMatchingGameplayTag(GameplayTags::State::Ghost::Invisible);
			bool bIsRevealed = MyASC->HasMatchingGameplayTag(GameplayTags::State::Ghost::Revealed);

			// 유령 모드 판정
			if (bIsInvisible && !bIsRevealed)
			{
				// 나 자신 숨기기
				CurrentActiveCapture->HiddenActors.Add(MyChar);

				// 내 부착물(무기 등)도 함께 숨김
				TArray<AActor*> MyAttached;
				MyChar->GetAttachedActors(MyAttached);
				for (AActor* Attached : MyAttached)
				{
					if (Attached) CurrentActiveCapture->HiddenActors.Add(Attached);
				}
			}
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

	// BindGameStateDelegates();

	UISubsystem->SetDefaultInputPolicy(EUIInputPolicy::GameOnly);

	BindGameStateDelegates();

	/* 게임설정 */
	if (UPTWGameUserSettings* Settings = Cast<UPTWGameUserSettings>(UGameUserSettings::GetGameUserSettings()))
	{
		CurrentMouseSensitivity = Settings->MouseSensitivity;
	}

	CreateUI();
	
	UE_LOG(LogTemp, Warning, TEXT("[PTWPlayerController] BeginPlay - CreateUI 함수 호출됨."));
}

void APTWPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		GetWorldTimerManager().ClearTimer(RespawnTimerHandle);
		GetWorldTimerManager().ClearTimer(GameStateBindRetryHandle);
	}
	
	Super::EndPlay(EndPlayReason);
	
	UE_LOG(LogTemp, Warning, TEXT("[PTWPlayerController] EndPlay 함수 호출됨."));
}

void APTWPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (!IsLocalController())
	{
		return;
	}

	/* ASC 등록 */
	APTWPlayerState* PS = GetPlayerState<APTWPlayerState>();
	if (PS)
	{
		ASC = PS->GetAbilitySystemComponent();
	}

	BindGameStateDelegates();

	//CreateUI();
	
	UE_LOG(LogTemp, Warning, TEXT("[PTWPlayerController] OnRep_PlayerState 함수 호출됨."));
}

void APTWPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();
	UE_LOG(LogTemp, Warning, TEXT("[PTWPlayerController] OnRep_Pawn 함수 호출됨."));

	CreateUI();
	UE_LOG(LogTemp, Warning, TEXT("[PTWPlayerController] OnRep_Pawn - CreateUI 함수 호출됨."));
}

void APTWPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	/* 로딩완료 */
	Server_ReportLoadingComplete();
	
	UE_LOG(LogTemp, Warning, TEXT("[PTWPlayerController] 플레이어 컨트롤러 Possess 함수 호출됨."));

	CreateUI();
	UE_LOG(LogTemp, Warning, TEXT("[PTWPlayerController] OnPossess - CreateUI 함수 호출됨."));
}

void APTWPlayerController::OnUnPossess()
{
	Super::OnUnPossess();
	UE_LOG(LogTemp, Warning, TEXT("[PTWPlayerController] OnUnPossess 함수 호출됨."));
}

void APTWPlayerController::BeginSpectatingState()
{
	Super::BeginSpectatingState();
	
	UE_LOG(LogTemp, Warning, TEXT("[PTWPlayerController] BeginSpectatingState 함수 호출됨."));
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

void APTWPlayerController::NotifyLoadedWorld(FName WorldPackageName, bool bFinalDest)
{
	Super::NotifyLoadedWorld(WorldPackageName, bFinalDest);

	if (bFinalDest && IsLocalController())
	{
		Server_NotifyMapLoaded();
		UE_LOG(LogTemp, Warning, TEXT("[PTWPlayerController] Server_NotifyMapLoaded 함수 호출됨."));
	}
}

void APTWPlayerController::BindGameStateDelegates()
{
	APTWGameState* GS = GetWorld() ? GetWorld()->GetGameState<APTWGameState>() : nullptr;
	if (!GS)
	{
		// 아직 GameState 안 왔으면 0.2초 후 재시도
		GetWorldTimerManager().SetTimer(
			GameStateBindRetryHandle,
			this,
			&APTWPlayerController::BindGameStateDelegates,
			0.2f,
			false
		);
		return;
	}
	GetWorldTimerManager().ClearTimer(GameStateBindRetryHandle);

	// 중복 바인딩 방지
	UnbindGameStateDelegates();

	// 델리게이트 바인드
	GS->OnMiniGameCountdownChanged.AddDynamic(this, &ThisClass::OnMiniGameCountdownChanged);
	GS->OnRoulettePhaseChanged.AddDynamic(this, &ThisClass::HandleRoulettePhaseChanged);
	GS->OnGamePhaseChanged.AddDynamic(this, &ThisClass::HandleGamePhaseChanged);

	// 현재상태 반영
	OnMiniGameCountdownChanged(GS->IsMiniGameCountdown());
	HandleRoulettePhaseChanged(GS->GetRouletteData());
	HandleGamePhaseChanged(GS->GetCurrentGamePhase());
	
	Server_NotifyReadyToPlay();
	
}

void APTWPlayerController::UnbindGameStateDelegates()
{
	if (APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>())
	{
		GS->OnMiniGameCountdownChanged.RemoveAll(this);
		GS->OnRoulettePhaseChanged.RemoveAll(this);
		GS->OnGamePhaseChanged.RemoveAll(this);
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

void APTWPlayerController::HandleGamePhaseChanged(EPTWGamePhase CurrentGamePhase)
{
	const UEnum* EnumPtr = StaticEnum<EPTWGamePhase>();
	FString PhaseName = EnumPtr ? EnumPtr->GetNameStringByValue((int64)CurrentGamePhase) : TEXT("Invalid");

	if (!UISubsystem)
	{
		return;
	}

	switch (CurrentGamePhase)
	{
	case EPTWGamePhase::GameResult:
		if (RankingBoardClass)
		{
			UISubsystem->SetWidgetVisibility(RankingBoardClass, true);
			bAbleRankingBoard = false;
		}
		break;

	case EPTWGamePhase::MiniGameResult:
		if (RankingBoardClass)
		{
			UISubsystem->SetWidgetVisibility(RankingBoardClass, true);
			bAbleRankingBoard = false;
		}
		break;

	default:
		if (RankingBoardClass)
		{
			UISubsystem->SetWidgetVisibility(RankingBoardClass, false);
			bAbleRankingBoard = true;
		}		
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
		
		// 마이크 (V)
		EIC->BindAction(
			VoiceAction,
			ETriggerEvent::Started,
			this,
			&ThisClass::OnVoicePressed
		);
		EIC->BindAction(
			VoiceAction,
			ETriggerEvent::Completed,
			this,
			&ThisClass::OnVoiceReleased
		);

		// 개발자용 UI (F6)
		EIC->BindAction(
			DevWidgetAction,
			ETriggerEvent::Started,
			this,
			&APTWPlayerController::ToggleDevUI
		);
	}
}

void APTWPlayerController::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();

	UE_LOG(LogTemp, Warning, TEXT("[PTWPlayerController] PostSeamlessTravel 함수 호출됨."));

	// 로컬 컨트롤러인지 다시 확인
	if (IsLocalController())
	{
		// 서브시스템 캐싱 (레벨 이동 후 PC가 새로 생성될 수 있으므로 다시 할당)
		if (ULocalPlayer* LP = GetLocalPlayer())
		{
			UISubsystem = LP->GetSubsystem<UPTWUISubsystem>();
		}

		CreateUI();
		UE_LOG(LogTemp, Warning, TEXT("[PTWPlayerController] PostSeamlessTravel - CreateUI 함수 호출됨."));
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
			bAbleChat = true;
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
	if (bAbleChat)
	{
		/* 채팅창 새로 열기(Push) */
		UISubsystem->PushWidget(ChatInputClass, EUIInputPolicy::GameAndUI);

		/* ChatList를 찾아 활성화 */
		if (UPTWChatList* ChatList = Cast<UPTWChatList>(UISubsystem->GetOrCreateWidget(ChatListClass)))
		{
			ChatList->SetInteractionMode(true);
		}

		bAbleChat = false;
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

void APTWPlayerController::Server_NotifyMapLoaded_Implementation()
{
	if (APTWGameMode* GameMode = GetWorld()->GetAuthGameMode<APTWGameMode>())
	{
		GameMode->PlayerReadyToPlay(this);
	}
}

void APTWPlayerController::HandleBombOwnerChanged(APawn* NewOwnerPawn)
{
	if (!IsLocalController()) return;

	if (NewOwnerPawn == GetPawn())
	{
		ShowBombUI();
	}
	else
	{
		HideBombUI();
	}
}

void APTWPlayerController::ShowBombUI()
{
	if (!BombWarningWidgetClass) return;

	UISubsystem->SetWidgetVisibility(BombWarningWidgetClass, true);
}

void APTWPlayerController::HideBombUI()
{
	if (!BombWarningWidgetClass) return;

	UISubsystem->SetWidgetVisibility(BombWarningWidgetClass, false);
}

void APTWPlayerController::ToggleDevUI()
{
	if (DevWidgetInstance && DevWidgetInstance->IsInViewport())
	{
		DevWidgetInstance->RemoveFromParent();
		SetInputMode(FInputModeGameOnly());
		bShowMouseCursor = false;
	}
	else if (DevWidgetClass)
	{
		if (!DevWidgetInstance)
		{
			DevWidgetInstance = CreateWidget<UPTWDevWidget>(this, DevWidgetClass);
		}

		if (DevWidgetInstance)
		{
			DevWidgetInstance->AddToViewport(999);

			FInputModeGameAndUI InputMode;
			InputMode.SetWidgetToFocus(DevWidgetInstance->TakeWidget());
			SetInputMode(InputMode);
			bShowMouseCursor = true;
		}
	}
}

void APTWPlayerController::CaptureTargetPOV()
{
	if (CurrentActiveCapture)
	{
		CurrentActiveCapture->CaptureScene();
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

void APTWPlayerController::Client_SetAbyssDark_Implementation(bool bEnable)
{
	if (!GetWorld()) return;
	
	if (!CachedAbyssPP)
	{
		for (TActorIterator<APostProcessVolume> It(GetWorld()); It; ++It)
		{
			if (It->ActorHasTag(FName("AbyssPP")))
			{
				CachedAbyssPP = *It;
				break;
			}
		}
	}

	if (!CachedAbyssPP) return;
	
	CachedAbyssPP->bEnabled = true;
	CachedAbyssPP->BlendWeight = bEnable ? 1.0f : 0.0f;
}
