// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWDevWidget.generated.h"

class UPTWButton;
class UEditableTextBox;

UCLASS()
class PTW_API UPTWDevWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> Input_Gold;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> Input_Time;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> Input_Speed;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> Input_Jump;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> Input_ItemID;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UPTWButton> Btn_AddBot;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UPTWButton> Btn_RemoveBot;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UPTWButton> Btn_TeleportBots;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UPTWButton> Btn_AddGold;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UPTWButton> Btn_SetTime;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UPTWButton> Btn_ForceStart;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UPTWButton> Btn_ForceEnd;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UPTWButton> Btn_ForceWin;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UPTWButton> Btn_ForceLose;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UPTWButton> Btn_GodMode;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UPTWButton> Btn_SuperSpeed;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UPTWButton> Btn_SuperJump;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UPTWButton> Btn_FlyMode;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UPTWButton> Btn_KillSelf;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UPTWButton> Btn_GrantItem;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UPTWButton> Btn_ShowFPS;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UPTWButton> Btn_ToggleHUD;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UPTWButton> Btn_ShowCollision;

	UFUNCTION() void OnAddBotClicked();
	UFUNCTION() void OnRemoveBotClicked();
	UFUNCTION() void OnTeleportBotsClicked();

	UFUNCTION() void OnAddGoldClicked();
	UFUNCTION() void OnSetTimeClicked();
	UFUNCTION() void OnForceStartClicked();
	UFUNCTION() void OnForceEndClicked();
	UFUNCTION() void OnForceWinClicked();
	UFUNCTION() void OnForceLoseClicked();

	UFUNCTION() void OnGodModeClicked();
	UFUNCTION() void OnSuperSpeedClicked();
	UFUNCTION() void OnSuperJumpClicked();
	UFUNCTION() void OnFlyModeClicked();
	UFUNCTION() void OnKillSelfClicked();

	UFUNCTION() void OnGrantItemClicked();
	UFUNCTION() void OnShowFPSClicked();
	UFUNCTION() void OnToggleHUDClicked();
	UFUNCTION() void OnShowCollisionClicked();
};
