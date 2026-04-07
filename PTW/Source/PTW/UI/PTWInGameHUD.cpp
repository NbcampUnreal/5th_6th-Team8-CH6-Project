// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWInGameHUD.h"
#include "AbilitySystemComponent.h" // ASC 

#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "CoreFramework/PTWPlayerState.h"

#include "InGameUI/PTWHealthBar.h"
#include "InGameUI/PTWKillLogUI.h"
#include "InGameUI/PTWTimer.h"
#include "InGameUI/PTWAmmoWidget.h"
#include "InGameUI/PTWCrosshair.h"
#include "InGameUI/PTWInventoryWidget.h"
#include "InGameUI/PTWMiniGameInventory.h"
#include "InGameUI/PTWNotificationWidget.h"
#include "InGameUI/PTWMiniGameTitle.h"
#include "InGameUI/PTWPortalCount.h"
#include "Inventory/PTWInventoryComponent.h"

void UPTWInGameHUD::InitializeUI(UAbilitySystemComponent* ASC)
{
	if (!ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("UPTWInGameHUD: ASC is null."));
		return;
	}
	UE_LOG(LogTemp, Error, TEXT("UPTWInGameHUD: ASC."));

	BindGameState();

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
		if (APTWPlayerState* PS = GetOwningPlayerState<APTWPlayerState>())
		{
			if (UPTWInventoryComponent* Inventory =	PS->FindComponentByClass<UPTWInventoryComponent>())
			{
				MiniGameInventoryWidget->InitInventory(Inventory);
			}
		}
	}
	if (PortalCount) PortalCount->InitializeGameState();
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

void UPTWInGameHUD::NativeDestruct()
{
	GetWorld()->GetTimerManager().ClearTimer(GameStateBindTimerHandle);

	UnBindGameState();

	Super::NativeDestruct();
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
	if (InventoryWidget && MiniGameInventoryWidget)
	{
		switch (Phase)
		{
		case EPTWGamePhase::PostGameLobby:
			InventoryWidget->SetVisibility(ESlateVisibility::Visible);
			MiniGameInventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
			PortalCount->SetVisibility(ESlateVisibility::Visible);
			break;

		case EPTWGamePhase::MiniGame:
			InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
			MiniGameInventoryWidget->SetVisibility(ESlateVisibility::Visible);
			PortalCount->SetVisibility(ESlateVisibility::Collapsed);
			break;

		default:
			InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
			MiniGameInventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
			PortalCount->SetVisibility(ESlateVisibility::Collapsed);
			break;
		}
	}

	if (MiniGameTitle)
	{
		MiniGameTitle->UpdateTitleByPhase(Phase);
	}
}

void UPTWInGameHUD::HandleRoulettePhaseChanged(FPTWRouletteData RouletteData)
{
	APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>();
	if (!IsValid(GS)) return;

	EPTWGamePhase Phase = GS->GetCurrentGamePhase();

	if (MiniGameTitle)
	{
		if (Phase == EPTWGamePhase::PostGameLobby)
		{
			MiniGameTitle->UpdateTitleByRoulette(RouletteData);
		}
	}
}

void UPTWInGameHUD::BindGameState()
{
	APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>();
	if (!IsValid(GS))
	{
		GetWorld()->GetTimerManager().SetTimer(GameStateBindTimerHandle, this, &ThisClass::BindGameState, 0.1f, false);
		return;
	}
	GetWorld()->GetTimerManager().ClearTimer(GameStateBindTimerHandle);

	UnBindGameState();

	GS->OnGamePhaseChanged.AddDynamic(this, &ThisClass::HandleGamePhaseChanged);
	GS->OnRoulettePhaseChanged.AddDynamic(this, &ThisClass::HandleRoulettePhaseChanged);

	// 초기화 시점 동기화
	HandleGamePhaseChanged(GS->GetCurrentGamePhase());
	HandleRoulettePhaseChanged(GS->GetRouletteData());
}

void UPTWInGameHUD::UnBindGameState()
{
	APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>();
	if (IsValid(GS))
	{
		GS->OnGamePhaseChanged.RemoveDynamic(this, &ThisClass::HandleGamePhaseChanged);
		GS->OnRoulettePhaseChanged.RemoveDynamic(this, &ThisClass::HandleRoulettePhaseChanged);
	}
}
