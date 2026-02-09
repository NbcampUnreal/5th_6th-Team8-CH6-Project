// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWInGameHUD.h"
#include "AbilitySystemComponent.h" // ASC 

#include "InGameUI/PTWHealthBar.h"
#include "InGameUI/PTWKillLogUI.h"
#include "InGameUI/PTWTimer.h"
#include "InGameUI/PTWAmmoWidget.h"
#include "InGameUI/PTWCrosshair.h"
#include "InGameUI/PTWInventoryWidget.h"

void UPTWInGameHUD::InitializeUI(UAbilitySystemComponent* ASC)
{
	if (!ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("UPTWInGameHUD: ASC is null."));
		return;
	}
	UE_LOG(LogTemp, Error, TEXT("UPTWInGameHUD: ASC."));

	/* HealthBar 초기화 */
	if (HealthBar) HealthBar->InitWithASC(ASC);
	/* AmmoWidget 초기화*/
	if (AmmoWidget) AmmoWidget->InitWithASC(ASC);
	/* 크로스헤어 초기화 */
	if (CrosshairWidget) CrosshairWidget->InitWithASC(ASC);
	/* 인벤토리 위젯 초기화 */
	if (InventoryWidget) InventoryWidget->InitPS();
}

bool UPTWInGameHUD::Initialize()
{
	bool Success = Super::Initialize();
	if (!Success) return false;

	return true;
}
