// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "UI/InGameUI/PTWNotificationWidget.h"
#include "PTWUIControllerComponent.generated.h"

class APTWPlayerController;
class UPTWUISubsystem;
class UUserWidget;
class UPTWDevWidget;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PTW_API UPTWUIControllerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPTWUIControllerComponent();

	void InitializeUIComponent(APTWPlayerController* InPC);
	void CreateUI();
	void ReInitializeUI();

	void ToggleRankingBoard(bool bShow);
	void TogglePauseMenu();
	void ToggleChat();
	void ToggleKeyGuide();
	void ToggleDevUI();
	void ChatInputFinished();

	void UpdateCountdownUI(bool bStarted);
	void UpdateRoulettePhaseUI(FPTWRouletteData RouletteData);
	void UpdateGamePhaseUI(EPTWGamePhase CurrentGamePhase);
	void ShowNotification(const FNotificationData& Data);
	void UpdateTargetPOV(APawn* NewTarget);
	void RefreshTargetViewHiddenActors();
	void ShowDamageIndicator(FVector DamageCauserLocation);
	void BuyVoteItem();
	
	UFUNCTION(Client, Reliable)
	void Client_OpenPredictVoteUI();

	/* 알림 위젯 */
	UFUNCTION(Client, Reliable)
	void Client_ShowNotification(const FNotificationData& Data);
	void ShowLocalNotification(const FNotificationData& Data);
	void SendMessage(
		const FText& InText,
		ENotificationPriority InPriority = ENotificationPriority::Normal,
		float InDuration = 2.f,
		bool bInterrupt = false);

	void Popup(const FText& InText);

	/* GameState 델리게이트 바인딩 */
	void BindGameStateDelegates();
	void UnbindGameStateDelegates();

	/* 리팩토링 필요 */
	/* 폭탄넘기기 미니게임 BombActor 델리게이트 바인딩 */
	//void BindBombDelegate(APTWBombActor* NewBomb);
	//void UnBindBombDelegate();

	/* GhostChase */
	UFUNCTION(Client, Reliable)
	void Client_FindGhostChaseComponent();

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void UpdateNameTagsVisibility();

	UFUNCTION()
	void OnMiniGameCountdownChanged(bool bStarted);

	/* GameState의 룰렛 상태 변경 델리게이트 수신 함수 */
	UFUNCTION()
	void HandleRoulettePhaseChanged(FPTWRouletteData RouletteData);

	/* 페이즈 변경 델리게이트 수신 함수 */
	UFUNCTION()
	void HandleGamePhaseChanged(EPTWGamePhase CurrentGamePhase);

	void HandleOpenPredictVoteUI();

public:
	UPROPERTY(EditDefaultsOnly, Category = "UI|NameTag") 
	float NameTagMaxDistance = 1500.f;
	UPROPERTY(EditDefaultsOnly, Category = "UI|NameTag") 
	float NameTagUpdateInterval = 0.1f;
	UPROPERTY(EditDefaultsOnly, Category = "UI|NameTag") 
	float NameTagMinScale = 0.4f;

	UPROPERTY(EditDefaultsOnly, Category = "UI") 
	TSubclassOf<class UPTWInGameHUD> HUDClass;
	UPROPERTY(EditDefaultsOnly, Category = "UI") 
	TSubclassOf<class UPTWRankingBoard> RankingBoardClass;
	UPROPERTY(EditDefaultsOnly, Category = "UI") 
	TSubclassOf<UUserWidget> PauseMenuClass;
	UPROPERTY(EditDefaultsOnly, Category = "UI|Chat") 
	TSubclassOf<class UPTWChatList> ChatListClass;
	UPROPERTY(EditDefaultsOnly, Category = "UI|Chat") 
	TSubclassOf<class UPTWChatInput> ChatInputClass;
	UPROPERTY(EditDefaultsOnly, Category = "UI") 
	TSubclassOf<class UPTWDamageIndicator> DamageIndicatorClass;
	UPROPERTY(EditDefaultsOnly, Category = "UI|Timer") 
	TSubclassOf<class UPTWGameStartTimer> GameStartTimerClass;
	UPROPERTY(EditDefaultsOnly, Category = "UI|Roulette") 
	TSubclassOf<UUserWidget> MapRouletteWidgetClass;
	UPROPERTY(EditDefaultsOnly, Category = "UI") 
	TSubclassOf<UUserWidget> KeyGuideWidgetClass;
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> PopupWidgetClass;
	UPROPERTY(EditAnywhere, Category = "UI") 
	TSubclassOf<UUserWidget> SpectatorHUDClass;
	UPROPERTY(EditAnywhere, Category = "UI") 
	TSubclassOf<class UPTWDevWidget> DevWidgetClass;
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> PredictWinVote;
	// 폭탄 경고 위젯
	UPROPERTY(EditDefaultsOnly, Category = "UI|Bomb")
	TSubclassOf<class UPTWBombWarning> BombWarningWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<APTWPlayerController> OwnerPC;
	UPROPERTY()
	TObjectPtr<UPTWUISubsystem> UISubsystem;
	UPROPERTY()
	TObjectPtr<class APTWGameState> CachedGameState;

	bool bAbleRankingBoard = false;
	bool bAbleChat = false;
	bool bKeyGuideOn = false;

	FTimerHandle NameTagTimerHandle;
	FTimerHandle GameStateBindRetryHandle;

	UPROPERTY()
	UPTWDevWidget* DevWidgetInstance;

	UPROPERTY()
	class UPTWGhostChaseControllerComponent* CachedGhostChaseComp;
};
