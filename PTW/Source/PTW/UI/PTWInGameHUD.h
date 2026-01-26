// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWInGameHUD.generated.h"

class UAbilitySystemComponent;

class UPTWHealthBar;
class UPTWKillLogUI;
class UPTWTimer;
class UPTWAmmoWidget;
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

	// 킬로그
	UFUNCTION()
	void AddKillLog(const FString& Killer, const FString& Victim); // 인자에 무기 종류 추가해야함
	//탄약
	void UpdateAmmo(int32 CurrentAmmo, int32 MaxAmmo);

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

protected:
	virtual bool Initialize() override;
};
