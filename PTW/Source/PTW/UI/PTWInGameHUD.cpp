// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWInGameHUD.h"
#include "AbilitySystemComponent.h" // ASC 

#include "InGameUI/PTWHealthBar.h"
#include "InGameUI/PTWKillLogUI.h"
#include "InGameUI/PTWTimer.h"
#include "InGameUI/PTWAmmoWidget.h"
#include "InGameUI/PTWCrosshair.h"
#include "InGameUI/PTWInventoryWidget.h"
#include "InGameUI/PTWNotificationWidget.h"

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

void UPTWInGameHUD::ShowNotification(const FNotificationData& Data)
{
	if (!NotificationWidget) return;

	if (bIsShowingNotification)
	{
		if (Data.bInterrupt)
		{
			NotificationWidget->ForceHide();
			bIsShowingNotification = false;
		}
		else
		{
			NotificationQueue.Add(Data);
			NotificationQueue.Sort([](const FNotificationData& A, const FNotificationData& B)
				{
					return (uint8)A.Priority > (uint8)B.Priority;
				});
			return;
		}
	}

	NotificationQueue.Add(Data);

	NotificationQueue.Sort([](const FNotificationData& A, const FNotificationData& B)
		{
			return (uint8)A.Priority > (uint8)B.Priority;
		});

	TryShowNextNotification();
}

bool UPTWInGameHUD::Initialize()
{
	bool Success = Super::Initialize();
	if (!Success) return false;

	if (NotificationWidget)
	{
		NotificationWidget->SetVisibility(ESlateVisibility::Collapsed);

		NotificationWidget->OnMessageFinished.AddUObject(
			this,
			&ThisClass::HandleNotificationFinished
		);
	}

	return true;
}

void UPTWInGameHUD::TryShowNextNotification()
{
	if (!NotificationWidget) return;

	if (NotificationQueue.Num() == 0)
	{
		bIsShowingNotification = false;
		return;
	}

	bIsShowingNotification = true;

	FNotificationData Data = NotificationQueue[0];
	NotificationQueue.RemoveAt(0);

	NotificationWidget->SetVisibility(ESlateVisibility::Visible);
	NotificationWidget->PlayMessage(Data);
}

void UPTWInGameHUD::HandleNotificationFinished()
{
	if (NotificationWidget)
	{
		NotificationWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	bIsShowingNotification = false;

	TryShowNextNotification();
}
