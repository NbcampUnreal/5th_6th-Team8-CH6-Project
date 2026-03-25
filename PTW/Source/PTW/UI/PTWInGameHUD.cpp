// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWInGameHUD.h"
#include "AbilitySystemComponent.h" // ASC 

#include "CoreFramework/Game/GameState/PTWGameState.h"

#include "InGameUI/PTWHealthBar.h"
#include "InGameUI/PTWKillLogUI.h"
#include "InGameUI/PTWTimer.h"
#include "InGameUI/PTWAmmoWidget.h"
#include "InGameUI/PTWCrosshair.h"
#include "InGameUI/PTWInventoryWidget.h"
#include "InGameUI/PTWMiniGameInventory.h"
#include "InGameUI/PTWNotificationWidget.h"
#include "InGameUI/PTWMiniGameTitle.h"

#include "Inventory/PTWInventoryComponent.h"

void UPTWInGameHUD::InitializeUI(UAbilitySystemComponent* ASC)
{
	if (!ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("UPTWInGameHUD: ASC is null."));
		return;
	}
	UE_LOG(LogTemp, Error, TEXT("UPTWInGameHUD: ASC."));

	BindGamePhase();

	/* Timer 초기화 */
	if (Timer) Timer->InitTimer();
	/* HealthBar 초기화 */
	if (HealthBar) HealthBar->InitWithASC(ASC);
	/* AmmoWidget 초기화*/
	if (AmmoWidget) AmmoWidget->InitWithASC(ASC);
	/* 크로스헤어 초기화 */
	if (CrosshairWidget) CrosshairWidget->InitWithASC(ASC);
	/* 인벤토리 위젯 초기화 */
	if (InventoryWidget) InventoryWidget->InitPS();
	/* 미니게임인벤토리 위젯 초기화 */
	if (MiniGameInventoryWidget)
	{
		if (APawn* Pawn = GetOwningPlayerPawn())
		{
			if (UPTWInventoryComponent* Inventory =
				Pawn->FindComponentByClass<UPTWInventoryComponent>())
			{
				MiniGameInventoryWidget->InitInventory(Inventory);
			}
		}
	}
	/* 인벤토리 위젯 초기화 */
	if (MiniGameTitle) MiniGameTitle->InitPS();
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

void UPTWInGameHUD::HandleGamePhaseChanged(EPTWGamePhase Phase)
{
	if (!InventoryWidget || !MiniGameInventoryWidget)
		return;

	switch (Phase)
	{
	case EPTWGamePhase::PostGameLobby:
	{
		InventoryWidget->SetVisibility(ESlateVisibility::Visible);
		MiniGameInventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
		break;
	}

	case EPTWGamePhase::MiniGame:
	{
		InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
		MiniGameInventoryWidget->SetVisibility(ESlateVisibility::Visible);
		break;
	}

	default:
	{
		InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
		MiniGameInventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
		break;
	}
	}
}

void UPTWInGameHUD::BindGamePhase()
{
	APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>();
	if (!GS) return;

	GS->OnGamePhaseChanged.AddDynamic(
		this,
		&UPTWInGameHUD::HandleGamePhaseChanged
	);

	// 현재 상태 즉시 동기화
	HandleGamePhaseChanged(GS->GetCurrentGamePhase());
}
