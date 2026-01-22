// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWInGameHUD.generated.h"

class UAbilitySystemComponent;

class UPTWHealthBar;

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

	/* 위젯 바인딩 */
	/* 체력바 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPTWHealthBar> HealthBar;

protected:
	virtual bool Initialize() override;
};
