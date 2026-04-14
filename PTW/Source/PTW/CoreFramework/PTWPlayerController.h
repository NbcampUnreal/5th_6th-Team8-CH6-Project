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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpectateTargetChanged, const FString&, TargetName);

class APTWPlayerState;
class UAbilitySystemComponent;
class UInputMappingContext;
class UInputAction;
class UPTWItemInstance;
class UPTWUISubsystem;
class APTWBombActor;
class UPTWBombWarning;
class UPTWDevWidget;
class UPTWDeveloperComponent;
class APostProcessVolume;
class UPTWAbyssControllerComponent;
class UGameplayEffect;
class UPTWGhostChaseControllerComponent;
class UPTWUIControllerComponent;

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

	void ApplyInputRestricted(bool bRestricted);
	
	/* 어비스 화면 연출 */
	UFUNCTION(Client, Reliable)
	void Client_SetAbyssDark(bool bEnable);
	
	/* 캐릭터 외곽선 */
	UFUNCTION(Client, Reliable)
	void Client_RefreshTeamOutline(bool bEnable, bool bUseTeam, bool bFriendlyOnly);
	
	/* 클라이언트가 서버에 메시지 전송을 요청하는 RPC */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendChatMessage(const FString& Message);

	/* 플레이어가 레벨 이동 후 준비 완료를 알리는 RPC */
	UFUNCTION(Server, Reliable)
	void Server_NotifyReadyToPlay();
	
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

	/* (폭탄넘기기 미니게임) BombActor 델리게이트 바인딩 */
	void BindBombDelegate(APTWBombActor* NewBomb);
	void UnBindBombDelegate();

	void OnVoicePressed();
	void OnVoiceReleased();

	/* UI 생성 */
	virtual void CreateUI();
	void ReInitializeUI();

	void SendMessage(
		const FText& InText,
		ENotificationPriority InPriority = ENotificationPriority::Normal,
		float InDuration = 2.f,
		bool bInterrupt = false);

	void Popup(const FText& InText);

	/* ControllerComponent 저장 */
	void SetControllerComponent(UActorComponent* NewControllerComponent);
	
	/* UISubsystem Getter*/
	FORCEINLINE UPTWUISubsystem* GetUISubSystem() const {return UISubsystem;}
	
	/* Controller Component Getter*/
	FORCEINLINE UActorComponent* GetControllerComponent() const {return BaseControllerComponent;}

	FORCEINLINE FString GetPlayerSessionId() const { return PlayerSessionId; };
	FORCEINLINE void SetPlayerSessionId(const FString& NewPlayerSessionId) { PlayerSessionId = NewPlayerSessionId ; };
	
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
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void SetupInputComponent() override;
	virtual void PostSeamlessTravel() override;

	/* 랭킹보드 (Tab) */
	void OnRankingPressed();
	void OnRankingReleased();

	/* PauseMenu (ESC) */
	void HandleMenuInput();

	/* 채팅창 (Enter) */
	void OnChatPressed();

	/* 키가이드 (K) */
	void OnKeyGuidePressed();

	/*  클라 로딩완료 알리기 */
	//UFUNCTION(Server, Reliable)
	//void Server_NotifyMapLoaded();
	
	/* (로딩스크린) 로딩완료 알리기 */
	UFUNCTION(Server, Reliable)
	void Server_ReportLoadingComplete();

	/* (폭탄넘기기 미니게임) 폭발 경고 UI */
	void HandleBombOwnerChanged(APawn* NewOwnerPawn);
	void ShowBombUI();
	void HideBombUI();
	
	/* 개발자용 UI 토글 */
	void ToggleDevUI();

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

	/* UI 컨트롤러 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UPTWUIControllerComponent> UIControllerComponent;

	FOnSpectateTargetChanged OnSpectateTargetChanged;
	
protected:
	// GameLift 접속을 위한 PlayerSessionId를 캐싱
	FString PlayerSessionId;
	
	/* 캐싱된 Ability System Component */
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;

	/* 캐싱된 UI 서브시스템 */
	UPROPERTY()
	TObjectPtr<UPTWUISubsystem> UISubsystem;

	/* (폭탄넘기기 미니게임) 캐싱 */
	UPROPERTY()
	APTWBombActor* CachedBombActor; // 폭탄 액터
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Abyss")
	TObjectPtr<UPTWAbyssControllerComponent> AbyssControllerComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UPTWGhostChaseControllerComponent> GhostChaseComponent;

	/* 각기 다른 미니 게임에서 사용하는 액터 컴퍼넌트 저장*/
	UPROPERTY(Replicated,VisibleAnywhere)
	TObjectPtr<UActorComponent> BaseControllerComponent;
	
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
	// 폭탄 경고 위젯
	UPROPERTY(EditDefaultsOnly, Category = "UI|Bomb")
	TSubclassOf<UPTWBombWarning> BombWarningWidgetClass;
	
};
