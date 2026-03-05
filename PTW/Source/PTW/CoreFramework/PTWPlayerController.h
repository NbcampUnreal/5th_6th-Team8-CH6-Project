// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "UI/InGameUI/PTWNotificationWidget.h"
#include "PTWPlayerController.generated.h"

/* KillLog 델리게이트 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnKillLog, const FString&, const FString&);

class UPTWInGameHUD;
class APTWPlayerState;
class UAbilitySystemComponent;
class UInputMappingContext;
class UInputAction;
class UPTWRankingBoard;
class UPTWItemInstance;
class UPTWUISubsystem;
class UPTWDamageIndicator;
class UPTWChatList;
class UPTWChatInput;
class UPTWGameStartTimer;
class APTWBombActor;
class UPTWBombWarning;
class UPTWDevWidget;
class UPTWDeveloperComponent;
class APostProcessVolume;
class UPTWTargetViewWidget;
class USceneCaptureComponent2D; 
class UTextureRenderTarget2D;   
/**
 * 
 */
UCLASS()
class PTW_API APTWPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	APTWPlayerController();

	/* 관전 시스템 함수 */
	void StartSpectating();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_StartSpectating();
	
	/* 데미지 인디케이터 */
	UFUNCTION(Client, Reliable)
	void ClientRPC_ShowDamageIndicator(FVector DamageCauserLocation);

	/* 입력 제한 함수 */
	UFUNCTION(Client, Reliable)
	void Client_SetInputRestricted(bool bRestricted);
	
	/*  화면 어둡게 토글 */
	UFUNCTION(Client, Reliable)
	void Client_SetAbyssDark(bool bEnable);

	void ApplyInputRestricted(bool bRestricted);
	
	/* 클라이언트가 서버에 메시지 전송을 요청하는 RPC */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendChatMessage(const FString& Message);
	/* 서버가 모든 클라이언트에게 메시지를 전송하는 RPC */
	//UFUNCTION(NetMulticast, Reliable)
	//void Multicast_BroadcastChatMessage(const FString& Sender, const FString& Message);
	/* 채팅창 종료 시 호출될 콜백 (ChatInput 위젯에서 호출) */
	void OnChatInputFinished();

	/* 게임설정 */
	void ApplyMouseSensitivity(float NewValue);

	/* 게임 로딩 관련 */
	UFUNCTION(Client, Reliable)
	void Client_PrepareLoadingScreen(ELoadingScreenType Type, FName MapRowName);
	UFUNCTION(Client, Reliable)
	void Client_DisplayLoadingScreen();

	/* 메인 메뉴로 이동 */
	UFUNCTION(Client, Reliable)
	void Client_OpenMainMenu();
	
	/* 서버에 플레이어가 준비 상태인 것을 알림 */
	UFUNCTION(Server, Reliable)
	void Server_NotifyReadyToPlay();

	/* (폭탄넘기기 미니게임) BombActor 델리게이트 바인딩 */
	void BindBombDelegate(APTWBombActor* NewBomb);
	void UnBindBombDelegate();

	void OnVoicePressed();
	void OnVoiceReleased();

	/* 알림 위젯 */
	UFUNCTION(Client, Reliable)
	void Client_ShowNotification(const FNotificationData& Data);
	void ShowLocalNotification(const FNotificationData& Data);
	void SendMessage(
		const FText& InText,
		ENotificationPriority InPriority = ENotificationPriority::Normal,
		float InDuration = 2.f,
		bool bInterrupt = false);

	/* 타겟 플레이어가 변경되었을 때 호출 */
	void UpdateTargetPOV(APawn* NewTarget);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnRep_PlayerState() override;
	virtual void OnRep_Pawn() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void BeginSpectatingState() override;
	virtual ASpectatorPawn* SpawnSpectatorPawn() override;
	virtual void NotifyLoadedWorld(FName WorldPackageName, bool bFinalDest) override;
	
	/*  ASC Delegate 바인딩 */
	//void BindASCDelegates();
	//void UnbindASCDelegates();

	/* GameState 델리게이트 바인딩 */
	void BindGameStateDelegates();
	void UnbindGameStateDelegates();

	/* 카운트다운 델리게이트 수신 함수 */
	UFUNCTION()
	void OnMiniGameCountdownChanged(bool bStarted);

	/* GameState의 룰렛 상태 변경 델리게이트 수신 함수 */
	UFUNCTION()
	void HandleRoulettePhaseChanged(FPTWRouletteData RouletteData);

	/* 페이즈 변경 델리게이트 수신 함수 */
	void HandleGamePhaseChanged(EPTWGamePhase CurrentGamePhase);

	virtual void SetupInputComponent() override;
	virtual void PostSeamlessTravel() override;

	/* UI 생성 */
	void CreateUI();

	/* 랭킹보드 (Tab) */
	void OnRankingPressed();
	void OnRankingReleased();

	/* PauseMenu (ESC) */
	void HandleMenuInput();

	/* 채팅창 (Enter) */
	void OnChatPressed();

	/* 키가이드 (K) */
	void OnKeyGuidePressed();

	/* 플레이어 이름 */
	/* 닉네임 가시성 업데이트 로직 */
	void UpdateNameTagsVisibility();

	/*  클라 로딩완료 알리기 */
	UFUNCTION(Server, Reliable)
	void Server_NotifyMapLoaded();
	
	/* (로딩스크린) 로딩완료 알리기 */
	UFUNCTION(Server, Reliable)
	void Server_ReportLoadingComplete();

	/* (폭탄넘기기 미니게임) 폭발 경고 UI */
	void HandleBombOwnerChanged(APawn* NewOwnerPawn);
	void ShowBombUI();
	void HideBombUI();
	
	/* 개발자용 UI 토글 */
	void ToggleDevUI();

	/* 타겟뷰 호출 */
	void CaptureTargetPOV();

public:
	/* KillLog 델리게이트 */
	FOnKillLog OnKillLog;
	
	/* 리스폰 타이머 핸들*/
	FTimerHandle RespawnTimerHandle;

	/* 게임설정 */
	float CurrentMouseSensitivity = 1.0f;

	/* 개발자용 액터 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UPTWDeveloperComponent* DeveloperComponent;

protected:
	/* 캐싱된 Ability System Component */
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;

	/* 캐싱된 UI 서브시스템 */
	UPROPERTY()
	TObjectPtr<UPTWUISubsystem> UISubsystem;
	
	/* Abyss: 캐싱된 PostProcessVolume */
	UPROPERTY(Transient)
	TObjectPtr<APostProcessVolume> CachedAbyssPP;

	/* (폭탄넘기기 미니게임) 캐싱 */
	UPROPERTY()
	APTWBombActor* CachedBombActor; // 폭탄 액터

	/* 씬 캡처 결과가 저장될 렌더 타겟 메모리 리소스 */
	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> TargetPOVRT;
	/* 현재 활성화되어 캡처를 수행 중인 대상의 캡처 컴포넌트 참조 (GC 방지를 위해 Strong Pointer 사용) */
	UPROPERTY()
	TObjectPtr<USceneCaptureComponent2D> CurrentActiveCapture;

	/* 게임스테이트 델리게이트 바인드용 */
	FTimerHandle GameStateBindRetryHandle;
	/* 닉네임 업데이트용 타이머 핸들 */
	FTimerHandle NameTagTimerHandle;
	/* 캡처 프레임 제한(30FPS)을 위한 타이머 핸들 */
	FTimerHandle POVCaptureTimerHandle;

	/* 위젯 Open 가능 유무 */
	bool bAbleRankingBoard; // 랭킹보드
	bool bAbleChat; // 채팅창

	/* 키가이드 토글 상태 */
	bool bKeyGuideOn;

	/* 닉네임 표시 제한거리 */
	UPROPERTY(EditDefaultsOnly, Category = "UI|NameTag")
	float NameTagMaxDistance = 1500.f;
	/* 닉네임 표시 업데이트 주기 */
	UPROPERTY(EditDefaultsOnly, Category = "UI|NameTag")
	float NameTagUpdateInterval = 0.1f; // 0.1초
	/* 닉네임 표시 크기 */
	UPROPERTY(EditDefaultsOnly, Category = "UI|NameTag")
	float NameTagMinScale = 0.4f; // 가장 멀리 있을 때의 최소 크기 (0.4)

	/* ---------- Input ---------- */
	// 랭킹보드 (Tab)
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* ShowRankingAction;

	// PauseMenu (ESC)
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> PauseMenuAction;

	// 채팅 (Enter)
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> ChattingAction;

	// 키가이드 (K)
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> KeyGuideAction;
	
	// 마이크 입력 (V)
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> VoiceAction;

	// 개발자용 UI (F8)
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> DevWidgetAction;
	
	/* ---------- UI ---------- */
	// HUD
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPTWInGameHUD> HUDClass;
	// 랭킹보드
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPTWRankingBoard> RankingBoardClass;
	// PauseMenu
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> PauseMenuClass; 
	// Chat
	UPROPERTY(EditDefaultsOnly, Category = "UI|Chat")
	TSubclassOf<UPTWChatList> ChatListClass;
	UPROPERTY(EditDefaultsOnly, Category = "UI|Chat")
	TSubclassOf<UPTWChatInput> ChatInputClass;
	// 데미지 인디케이터
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPTWDamageIndicator> DamageIndicatorClass;
	// 카운트다운 타이머
	UPROPERTY(EditDefaultsOnly, Category = "UI|Timer")
	TSubclassOf<UPTWGameStartTimer> GameStartTimerClass;
	// 룰렛
	UPROPERTY(EditDefaultsOnly, Category = "UI|Roulette")
	TSubclassOf<UUserWidget> MapRouletteWidgetClass;
	// 키가이드
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> KeyGuideWidgetClass;
	// 폭탄 경고 위젯
	UPROPERTY(EditDefaultsOnly, Category = "UI|Bomb")
	TSubclassOf<UPTWBombWarning> BombWarningWidgetClass;
	UPROPERTY(EditAnywhere, Category="UI") 
	TSubclassOf<UPTWDevWidget> DevWidgetClass;
	UPROPERTY()
	UPTWDevWidget* DevWidgetInstance;
	// 타겟뷰 위젯
	UPROPERTY(EditDefaultsOnly, Category = "UI|GhostChase")
	TSubclassOf<UPTWTargetViewWidget> POVWidgetClass;
};
