// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/InGameUI/PTWNotificationWidget.h"
#include "PTW/CoreFramework/Game/GameState/PTWGameState.h"
#include "PTWInGameHUD.generated.h"

class UAbilitySystemComponent;

class UPTWHealthBar;
class UPTWKillLogUI;
class UPTWTimer;
class UPTWAmmoWidget;
class UPTWCrosshair;
class UPTWInventoryWidget;
class UPTWMiniGameInventory;
class UPTWNotificationWidget;
class UPTWMiniGameTitle;
class UPTWPortalCount;

/**
 * 
 */
UCLASS()
class PTW_API UPTWInGameHUD : public UUserWidget
{
	GENERATED_BODY()
	
public:
	/* 초기화 (HUD 에서 ASC 받고, 하위 위젯에 ASC 전달) */
	void InitializeUI(UAbilitySystemComponent* ASC);

	/* 알림 위젯에 문구 추가 */
	void ShowNotification(const FNotificationData& Data);

	/* 위젯 바인딩 */
	/* 체력바 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPTWHealthBar> HealthBar;
	/* 킬로그 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPTWKillLogUI> KillLogUI;
	/* 타이머 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPTWTimer> Timer;
	/* 탄약 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPTWAmmoWidget> AmmoWidget;
	/* 크로스헤어 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPTWCrosshair> CrosshairWidget;
	/* 인벤토리 위젯 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPTWInventoryWidget> InventoryWidget;
	/* 미니게임인벤토리 위젯 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPTWMiniGameInventory> MiniGameInventoryWidget;
	/* 알림 위젯 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPTWNotificationWidget> NotificationWidget;
	/* 미니게임이름 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPTWMiniGameTitle> MiniGameTitle;
	/* 포탈카운트 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPTWPortalCount> PortalCount;

protected:
	virtual void NativeDestruct() override;

	virtual bool Initialize() override;

	/* 알림 위젯 */
	void TryShowNextNotification();

	UFUNCTION()
	void HandleNotificationFinished();

	UFUNCTION()
	void HandleGamePhaseChanged(EPTWGamePhase Phase);
	UFUNCTION()
	void HandleRoulettePhaseChanged(FPTWRouletteData RouletteData);

	void BindGameState();
	void UnBindGameState();

	// GameState 대기용
	FTimerHandle GameStateBindTimerHandle;

	UPROPERTY()
	TArray<FNotificationData> NotificationQueue;

	bool bIsShowingNotification = false;
};
