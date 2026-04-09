// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWPlayerController.h"
#include "PTWPlayerState.h"

#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/GameState.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/SpectatorPawn.h"
#include "GameplayTagContainer.h"
#include "EngineUtils.h"
#include "Components/WidgetComponent.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Engine/PostProcessVolume.h"
#include "OnlineSubsystemUtils.h"

#include "PTWGameplayTag/GameplayTags.h"
#include "CoreFramework/PTWGameUserSettings.h"
#include "CoreFramework/PTWBaseCharacter.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "CoreFramework/Game/GameMode/PTWGameMode.h"
#include "CoreFramework/Game/GameInstance/PTWGameInstance.h"
#include "CoreFramework/Character/Component/PTWUIControllerComponent.h"
#include "CoreFramework/Character/Component/PTWDeveloperComponent.h"
#include "CoreFrameWork/Game/GameMode/PTWLobbyGameMode.h"
#include "UI/PTWUISubsystem.h"
#include "UI/PTWHUD.h"
#include "UI/PTWInGameHUD.h"
#include "UI/RankBoard/PTWRankingBoard.h"
#include "UI/ChatWidget/PTWChatList.h"
#include "UI/ChatWidget/PTWChatInput.h"
#include "UI/InGameUI/PTWDamageIndicator.h"
#include "UI/MiniGame/PTWGameStartTimer.h"
#include "UI/MiniGame/Bomb/PTWBombWarning.h"
#include "UI/Dev/PTWDevWidget.h"
#include "MiniGame/Item/BombItem/PTWBombActor.h"
#include "MiniGame/ControllerComponent/Abyss/PTWAbyssControllerComponent.h"
#include "MiniGame/ControllerComponent/GhostChase/PTWGhostChaseControllerComponent.h"
#include "Inventory/Instance/PTWItemInstance.h"
#include "System/PTWSteamSessionSubsystem.h"
#include "Weapon/PTWWeaponActor.h"

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
	
	if (UPTWSteamSessionSubsystem* SteamSessionSubsystem = UPTWSteamSessionSubsystem::Get(this))
	{
		SteamSessionSubsystem->LeaveGameSession();
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

	if (!BombWarningWidgetClass) return;

	UUserWidget* Widget = UISubsystem->ShowSystemWidget(BombWarningWidgetClass, 70);
	if (UPTWBombWarning* BombWidget = Cast<UPTWBombWarning>(Widget))
	{
		BombWidget->SetTargetBomb(CachedBombActor);
	}
	UISubsystem->SetWidgetVisibility(BombWarningWidgetClass, false);

	if (CachedBombActor->GetBombOwnerPawn())
	{
		HandleBombOwnerChanged(CachedBombActor->GetBombOwnerPawn());
	}
}

void APTWPlayerController::UnBindBombDelegate()
{
	if (CachedBombActor)
	{
		CachedBombActor->OnBombOwnerChanged.RemoveAll(this);
	}

	if (!BombWarningWidgetClass) return;

	UISubsystem->HideSystemWidget(BombWarningWidgetClass);
}

void APTWPlayerController::OnVoicePressed()
{
	if (IsLocalPlayerController())
	{
		if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
		{
			if (IOnlineVoicePtr VoiceInterface = Subsystem->GetVoiceInterface())
			{
				VoiceInterface->StartNetworkedVoice(0);
			}
		}
	}
}

void APTWPlayerController::OnVoiceReleased()
{
	if (IsLocalPlayerController())
	{
		if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
		{
			if (IOnlineVoicePtr VoiceInterface = Subsystem->GetVoiceInterface())
			{
				VoiceInterface->StopNetworkedVoice(0);
			}
		}
	}
}

void APTWPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController()) return;

	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		UISubsystem = LP->GetSubsystem<UPTWUISubsystem>();
	}

	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	UISubsystem->SetDefaultInputPolicy(EUIInputPolicy::GameOnly);

	if (UPTWGameUserSettings* Settings = Cast<UPTWGameUserSettings>(UGameUserSettings::GetGameUserSettings()))
	{
		CurrentMouseSensitivity = Settings->MouseSensitivity;
	}

	ReInitializeUI();
}

void APTWPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void APTWPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (!IsLocalController())
	{
		return;
	}

	APTWPlayerState* PS = GetPlayerState<APTWPlayerState>();
	if (PS)
	{
		ASC = PS->GetAbilitySystemComponent();
	}
}

void APTWPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	ReInitializeUI();
}

void APTWPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	Server_ReportLoadingComplete();

	ReInitializeUI();
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
				}
			}
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

	// 로컬 컨트롤러인지 다시 확인
	if (IsLocalController())
	{
		// 서브시스템 캐싱 (레벨 이동 후 PC가 새로 생성될 수 있으므로 다시 할당)
		if (ULocalPlayer* LP = GetLocalPlayer())
		{
			UISubsystem = LP->GetSubsystem<UPTWUISubsystem>();
		}

		ReInitializeUI();
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
void APTWPlayerController::ReInitializeUI()
{
	if (UIControllerComponent)
	{
		UIControllerComponent->InitializeUIComponent(this);
		UIControllerComponent->ReInitializeUI();
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

namespace PTWOutlineStencil
{
	static constexpr int32 None = 0;
	static constexpr int32 Friendly = 1;
	static constexpr int32 Enemy = 2;
}

void APTWPlayerController::Client_RefreshTeamOutline_Implementation(bool bEnable, bool bUseTeam, bool bFriendlyOnly)
{
	UWorld* World = GetWorld();
	if (!World) return;

	APTWPlayerState* LocalPS = GetPlayerState<APTWPlayerState>();
	APawn* LocalPawn = GetPawn();

	for (TActorIterator<APTWPlayerCharacter> It(World); It; ++It)
	{
		APTWPlayerCharacter* TargetCharacter = *It;
		if (!TargetCharacter) continue;

		APTWPlayerState* TargetPS = TargetCharacter->GetPlayerState<APTWPlayerState>();
		if (!TargetPS)
		{
			TargetCharacter->ClearOutlineStencil();
			continue;
		}

		if (!bEnable)
		{
			TargetCharacter->ClearOutlineStencil();
			continue;
		}

		if (bUseTeam)
		{
			const bool bIsSelf = (TargetCharacter == LocalPawn);
			const bool bIsFriendly = (LocalPS && LocalPS->GetTeamId() == TargetPS->GetTeamId());

			if (bFriendlyOnly)
			{
				//  아군만 표시
				if (!bIsSelf && bIsFriendly)
				{
					TargetCharacter->SetOutlineStencil(PTWOutlineStencil::Friendly);
				}
				else
				{
					TargetCharacter->ClearOutlineStencil();
				}
			}
			else
			{
				//  일반 팀전 (아군/적 둘 다)
				if (!bIsSelf)
				{
					TargetCharacter->SetOutlineStencil(
						bIsFriendly ? PTWOutlineStencil::Friendly : PTWOutlineStencil::Enemy
					);
				}
				else
				{
					TargetCharacter->ClearOutlineStencil();
				}
			}
		}
		else
		{
			//  개인전
			if (TargetCharacter == LocalPawn)
			{
				TargetCharacter->ClearOutlineStencil();
			}
			else
			{
				TargetCharacter->SetOutlineStencil(PTWOutlineStencil::Enemy);
			}
		}
	}
}
