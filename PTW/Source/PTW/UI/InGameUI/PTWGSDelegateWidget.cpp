// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGameUI/PTWGSDelegateWidget.h"
#include "UI/InGameUI/PTWInventoryWidget.h"
#include "UI/InGameUI/PTWMiniGameInventory.h"
#include "UI/InGameUI/PTWMiniGameTitle.h"
#include "Inventory/PTWInventoryComponent.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"

void UPTWGSDelegateWidget::InitializeDelegateWidget()
{
	if (InventoryWidget) InventoryWidget->InitPS();

	if (MiniGameInventoryWidget)
	{
		if (APawn* Pawn = GetOwningPlayerPawn())
		{
			if (UPTWInventoryComponent* Inventory = Pawn->FindComponentByClass<UPTWInventoryComponent>())
			{
				MiniGameInventoryWidget->InitInventory(Inventory);
			}
		}
	}

	TryBindGameState();
}

void UPTWGSDelegateWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InitializeDelegateWidget();
}

void UPTWGSDelegateWidget::NativeDestruct()
{
	GetWorld()->GetTimerManager().ClearTimer(GameStateBindTimerHandle);

	APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>();
	if (GS)
	{
		GS->OnGamePhaseChanged.RemoveDynamic(this, &ThisClass::HandleGamePhaseChanged);
		GS->OnRoulettePhaseChanged.RemoveDynamic(this, &ThisClass::HandleRoulettePhaseChanged);
	}

	Super::NativeDestruct();
}

void UPTWGSDelegateWidget::TryBindGameState()
{
	APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>();
	if (!GS)
	{
		GetWorld()->GetTimerManager().SetTimer(GameStateBindTimerHandle, this, &ThisClass::TryBindGameState, 0.1f, false);
		return;
	}

	GS->OnGamePhaseChanged.AddDynamic(this, &ThisClass::HandleGamePhaseChanged);
	GS->OnRoulettePhaseChanged.AddDynamic(this, &ThisClass::HandleRoulettePhaseChanged);

	// 초기화 시점 동기화
	HandleGamePhaseChanged(GS->GetCurrentGamePhase());
	HandleRoulettePhaseChanged(GS->GetRouletteData());
}

void UPTWGSDelegateWidget::HandleGamePhaseChanged(EPTWGamePhase Phase)
{
	if (InventoryWidget && MiniGameInventoryWidget)
	{
		switch (Phase)
		{
		case EPTWGamePhase::PostGameLobby:
			InventoryWidget->SetVisibility(ESlateVisibility::Visible);
			MiniGameInventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
			break;

		case EPTWGamePhase::MiniGame:
			InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
			MiniGameInventoryWidget->SetVisibility(ESlateVisibility::Visible);
			break;

		default:
			InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
			MiniGameInventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
			break;
		}
	}

	if (MiniGameTitle)
	{
		MiniGameTitle->UpdateTitleByPhase(Phase);
	}
}

void UPTWGSDelegateWidget::HandleRoulettePhaseChanged(FPTWRouletteData RouletteData)
{
	if (MiniGameTitle)
	{
		MiniGameTitle->UpdateTitleByRoulette(RouletteData);
	}
}
