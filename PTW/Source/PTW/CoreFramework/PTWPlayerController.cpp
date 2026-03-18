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
#include "CoreFramework/Character/Component/PTWUIControllerComponent.h"
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
#include "MiniGame/ControllerComponent/Abyss/PTWAbyssControllerComponent.h"
#include "MiniGame/ControllerComponent/GhostChase/PTWGhostChaseControllerComponent.h"
#include "Engine/PostProcessVolume.h"
#include "EngineUtils.h"
#include "Game/GameMode/PTWLobbyGameMode.h"
#include "PTWGameplayTag/GameplayTags.h"

APTWPlayerController::APTWPlayerController()
{
	DeveloperComponent = CreateDefaultSubobject<UPTWDeveloperComponent>(TEXT("DevComponent"));
	UIControllerComponent = CreateDefaultSubobject<UPTWUIControllerComponent>(TEXT("UIControllerComponent"));
	AbyssControllerComponent = CreateDefaultSubobject<UPTWAbyssControllerComponent>(TEXT("AbyssControllerComponent"));
	GhostChaseComponent = CreateDefaultSubobject<UPTWGhostChaseControllerComponent>(TEXT("GhostChaseComponent"));
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

	if (!IsLocalController()) return;

	/*if (UIControllerComponent)
	{
		UIControllerComponent->InitializeUIComponent(this);
	}*/

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

	UISubsystem->SetDefaultInputPolicy(EUIInputPolicy::GameOnly);

	/* 게임설정 */
	if (UPTWGameUserSettings* Settings = Cast<UPTWGameUserSettings>(UGameUserSettings::GetGameUserSettings()))
	{
		CurrentMouseSensitivity = Settings->MouseSensitivity;
	}

	CreateUI();
	UE_LOG(LogTemp, Warning, TEXT("[PTWPlayerController] %s 플레이어 BeginPlay - CreateUI 함수 호출됨."), 
		PlayerState ? *PlayerState->GetPlayerName() : TEXT("Unknown"));
}

void APTWPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	UE_LOG(LogTemp, Warning, TEXT("[PTWPlayerController] %s 플레이어 EndPlay 함수 호출됨."), 
		PlayerState ? *PlayerState->GetPlayerName() : TEXT("Unknown"));
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
}

void APTWPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();
	UE_LOG(LogTemp, Warning, TEXT("[PTWPlayerController] %s 플레이어 OnRep_Pawn 함수 호출됨."), 
		PlayerState ? *PlayerState->GetPlayerName() : TEXT("Unknown"));

	CreateUI();
	UE_LOG(LogTemp, Warning, TEXT("[PTWPlayerController] %s 플레이어 OnRep_Pawn - CreateUI 함수 호출됨."), 
		PlayerState ? *PlayerState->GetPlayerName() : TEXT("Unknown"));
}

void APTWPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	/* 로딩완료 */
	Server_ReportLoadingComplete();
	
	UE_LOG(LogTemp, Warning, TEXT("[PTWPlayerController] %s 플레이어 컨트롤러 Possess 함수 호출됨."), 
		PlayerState ? *PlayerState->GetPlayerName() : TEXT("Unknown"));

	CreateUI();
	UE_LOG(LogTemp, Warning, TEXT("[PTWPlayerController] %s 플레이어 OnPossess - CreateUI 함수 호출됨."), 
		PlayerState ? *PlayerState->GetPlayerName() : TEXT("Unknown"));
}

void APTWPlayerController::OnUnPossess()
{
	Super::OnUnPossess();
	UE_LOG(LogTemp, Warning, TEXT("[PTWPlayerController] %s 플레이어 OnUnPossess 함수 호출됨."), 
		PlayerState ? *PlayerState->GetPlayerName() : TEXT("Unknown"));
}

void APTWPlayerController::BeginSpectatingState()
{
	Super::BeginSpectatingState();
	
	UE_LOG(LogTemp, Warning, TEXT("[PTWPlayerController] %s 플레이어 BeginSpectatingState 함수 호출됨."), 
		PlayerState ? *PlayerState->GetPlayerName() : TEXT("Unknown"));
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
		//Server_NotifyMapLoaded();
		UE_LOG(LogTemp, Warning, TEXT("[PTWPlayerController] Server_NotifyMapLoaded 함수 호출됨."));
	}
}

void APTWPlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, BaseControllerComponent);
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
			&APTWPlayerController::OnVoicePressed
		);
		EIC->BindAction(
			VoiceAction,
			ETriggerEvent::Completed,
			this,
			&APTWPlayerController::OnVoiceReleased
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

	UE_LOG(LogTemp, Warning, TEXT("[PTWPlayerController] %s 플레이어 PostSeamlessTravel 함수 호출됨."), 
		PlayerState ? *PlayerState->GetPlayerName() : TEXT("Unknown"));

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

void APTWPlayerController::ClientRPC_ShowDamageIndicator_Implementation(FVector DamageCauserLocation)
{
	UIControllerComponent->ShowDamageIndicator(DamageCauserLocation);
}

void APTWPlayerController::OnChatInputFinished()
{
	UIControllerComponent->ChatInputFinished();
}

void APTWPlayerController::CreateUI() 
{
	if (UIControllerComponent)
	{
		UIControllerComponent->InitializeUIComponent(this);
		UIControllerComponent->CreateUI();
	}
}
void APTWPlayerController::OnRankingPressed() { UIControllerComponent->ToggleRankingBoard(true); }
void APTWPlayerController::OnRankingReleased() { UIControllerComponent->ToggleRankingBoard(false); }
void APTWPlayerController::HandleMenuInput() { UIControllerComponent->TogglePauseMenu(); }
void APTWPlayerController::OnChatPressed() { UIControllerComponent->ToggleChat(); }
void APTWPlayerController::OnKeyGuidePressed() { UIControllerComponent->ToggleKeyGuide(); }
void APTWPlayerController::ToggleDevUI() { UIControllerComponent->ToggleDevUI(); }

void APTWPlayerController::SendMessage(const FText& InText,ENotificationPriority InPriority, float InDuration, bool bInterrupt)
{
	UIControllerComponent->SendMessage(InText, InPriority, InDuration, bInterrupt);
}

void APTWPlayerController::Popup(const FText& InText)
{
	UIControllerComponent->Popup(InText);
}

void APTWPlayerController::SetControllerComponent(UActorComponent* NewControllerComponent)
{
	if (!HasAuthority()) return;
	BaseControllerComponent = NewControllerComponent;
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
	if (AbyssControllerComponent)
	{
		AbyssControllerComponent->SetAbyssDark(bEnable);
	}
}
