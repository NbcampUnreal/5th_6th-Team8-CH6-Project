// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
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
/**
 * 
 */
UCLASS()
class PTW_API APTWPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
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

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnRep_PlayerState() override;
	virtual void OnRep_Pawn() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void BeginSpectatingState() override;
	virtual void SetViewTarget(AActor* NewViewTarget, 
		FViewTargetTransitionParams TransitionParams = FViewTargetTransitionParams()) override;

	void SetOwnerNoSeeRecursive(USceneComponent* InParentComponent, bool bNewOwnerNoSee);
	void SetSetOnlyOwnerSeeRecursive(USceneComponent* InParentComponent, bool bNewOnlyOwnerSee);
	
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

	/* 플레이어 이름 */
	/* 닉네임 가시성 업데이트 로직 */
	void UpdateNameTagsVisibility();
	
public:
	/* KillLog 델리게이트 */
	FOnKillLog OnKillLog;
	
	/* 리스폰 타이머 핸들*/
	FTimerHandle RespawnTimerHandle;

	/* 게임설정 */
	float CurrentMouseSensitivity = 1.0f;

protected:
	/* 캐싱된 Ability System Component */
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;

	/* 캐싱된 UI 서브시스템 */
	UPROPERTY()
	TObjectPtr<UPTWUISubsystem> UISubsystem;

	/* 닉네임 업데이트용 타이머 핸들 */
	FTimerHandle NameTagTimerHandle;

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

};
